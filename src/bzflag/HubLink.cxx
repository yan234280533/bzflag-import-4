/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

// implementation header
#include "HubLink.h"

// system headers
#include <errno.h>
#include <stdarg.h>
#include <string>
#include <set>

// common headers
#include "AnsiCodes.h"
#include "AresHandler.h"
#include "BZDBCache.h"
#include "DirectoryNames.h"
#include "FileManager.h"
#include "HubComposeKey.h"
#include "HUDRenderer.h"
#include "HUDui.h"
#include "TextUtils.h"
#include "TimeKeeper.h"
#include "bzfio.h"
#include "bz_md5.h"
#include "network.h"
#include "version.h"

// local headers
#include "LuaHeader.h"
#include "guiplaying.h"
#include "playing.h"


HubLink* hubLink = NULL;

static const char* ThisLabel = "this";

static const std::string codeFileName = "hub.lua";

static BZDB_int debugHub("debugHub");


//============================================================================//
//============================================================================//

static void debugf(int level, const char* fmt, ...)
{
  if (level > debugHub) {
    return;
  }
  const std::string fmt2 = std::string("HubLink: ") + fmt;
  va_list ap;
  va_start(ap, fmt);
  logDebugMessageArgs(0, fmt2.c_str(), ap);
  va_end(ap);
}


//============================================================================//
//============================================================================//

HubLink::HubLink(const std::string& _hostPort,
                 const std::string& _luaCode)
: hostPort(_hostPort)
, state(StateInit)
, ares(NULL)
, L(NULL)
, sock(-1)
, recvTotal(0)
, sendTotal(0)
, luaCode(_luaCode)
, wantDisable(false)
{
  // do nothing
}


HubLink::~HubLink()
{
  clear();
}


void HubLink::clear()
{
  if (L != NULL) {
    shutdown();
    lua_close(L);
    L = NULL;
  }
  if (ares != NULL) {
    delete ares;
    ares = NULL;
  }
  if (sock >= 0) {
    close(sock);
    sock = -1;
  }
  if (controlPanel != NULL) {
    std::set<std::string>::const_iterator it;
    for (it = tabs.begin(); it != tabs.end(); ++it) {
      controlPanel->removeTab(*it);
    }
  }
  tabs.clear();
}


void HubLink::fail(const std::string& msg)
{
  state = StateFailed;
  error = msg;
  clear();
}


void HubLink::fail(const std::string& msg, int errnum)
{
  fail(msg + std::string(strerror(errnum)));
}


bool HubLink::parseHostPort(std::string& host, int& port)
{
  if (hostPort.empty()) {
    return false;
  }

  const std::string::size_type colon = hostPort.find(':');
  if (colon == std::string::npos) {
    host = hostPort;
    port = defaultPort;
    return true;
  }

  host = hostPort.substr(0, colon);
  if (hostPort.find(':', colon + 1) != std::string::npos) {
    return false;
  }

  const std::string portStr = hostPort.substr(colon + 1);
  char* endPtr;
  const char* startPtr = portStr.c_str();
  port = strtol(startPtr, &endPtr, 10);
  return (endPtr != startPtr);
}


//============================================================================//
//============================================================================//

std::string HubLink::getLuaCodeFilename() const
{
  return getConfigDirName(BZ_CONFIG_DIR_VERSION) + codeFileName;
}


bool HubLink::loadLuaCode(std::string& code) const
{
  code = "";

  FILE* file = fopen(getLuaCodeFilename().c_str(), "rb");
  if (file == NULL) {
    return false;
  }

  char buf[4096];
  while (true) {
    const int bytes = fread(buf, 1, sizeof(buf), file);
    if (bytes > 0) {
      code.append(buf, bytes);
    } else {
      break;
    }
  }
  fclose(file);

  return true;
}


bool HubLink::saveLuaCode(const std::string& code) const
{
  // open with binary mode and truncation enabled
  const std::string fileName = getLuaCodeFilename();
  std::ostream* out = FILEMGR.createDataOutStream(fileName, true, true);
  if (out == NULL) {
    return false;
  }
  *out << code;
  delete out;
  return true;
}


//============================================================================//

bool HubLink::update()
{
  switch (state) {
    case StateInit:      { stateInit();      break; }
    case StateDNS:       { stateDNS();       break; }
    case StateConnect:   { stateConnect();   break; }
    case StateGetScript: { stateGetScript(); break; }
    case StateReady:     { stateReady();     break; }
    case StateFailed:    { return false; }
  }

  if (state == StateFailed) {
    if (controlPanel) {
      controlPanel->addMessage("HubLink: " + error);
    }
    logDebugMessage(0, "HubLink: %s\n", error.c_str());
    return false;
  }

  return true;
}


void HubLink::stateInit()
{
  std::string host;
  int port;
  if (!parseHostPort(host, port)) {
    fail("bad server address/port");
    return;
  }
  debugf(1, "stateInit() host='%s' port=%i\n", host.c_str(), port);
  ares = new AresHandler();
  ares->queryHost(host.c_str());
  state = StateDNS;
}


void HubLink::stateDNS()
{
  int port;
  std::string host;
  if (!parseHostPort(host, port)) {
    fail("bad server address/port");
    return;
  }

  struct timeval timeout = { 0, 0 };
  int nfds = -1;
  fd_set rfds, wfds;
  FD_ZERO(&rfds);
  FD_ZERO(&wfds);
  ares->setFd(&rfds, &wfds, nfds);
  nfds = select(nfds + 1, &rfds, &wfds, NULL, &timeout);
  ares->process(&rfds, &wfds);

  struct sockaddr_in addr;
  AresHandler::ResolutionStatus status = ares->getHostAddress(&addr.sin_addr);
  if (status == AresHandler::Failed) {
    fail("DNS query failed for '" + host + "'");
    return;
  }

  if (status != AresHandler::HbNSucceeded) {
    return; // not done yet
  }

  // connect the socket
  delete ares;
  ares = NULL;

  sock = (int)socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    fail("socket() error: ", errno);
    return;
  }

  if (BzfNetwork::setNonBlocking(sock) != 0) {
    fail("setNonBlocking() error");
    return;
  }

  // add the rest of the address information
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port); // set the port
        
  const int connCode = connect(sock, (const sockaddr*)&addr, sizeof(addr));
  if ((connCode != 0) && (errno != EINPROGRESS)) {
    fail("connect() error: ", errno);
    return;
  }

  state = StateConnect;
}


void HubLink::stateConnect()
{
  struct timeval timeout = { 0, 0 };
  fd_set wfds;
  FD_ZERO(&wfds);
  FD_SET(sock, &wfds);
  const int selCode = select(sock + 1, NULL, &wfds, NULL, &timeout);
  if (selCode == -1) {
    fail("connect/select error: ", errno);
    return;
  }
  if (selCode == 0) {
    return; // still cookin'
  }

  int optVal;
  socklen_t optLen = (socklen_t)sizeof(optVal);
  const int optCode = getsockopt(sock, SOL_SOCKET, SO_ERROR, &optVal, &optLen);
  if (optCode == -1) {
    fail("connect/getsockopt error: ", errno);
    return;
  }
  if (optVal != 0) {
    fail("connect/getsockopt value error: ", optVal);
    return;
  }

  // NOTE:
  //   we skip StateGetScript if initialized with lua code.
  //   this also avoids sending the VERSION hello message
  //   (could be used to redirect to servers with any protocol)
  //
  // example lua code for redirection (starting at the first line):
  //   bz.Reload('jabber.net', '--' .. bz.GetCode())
  //   -- ... (implement the Jabber protocol here)
  //
  if (!luaCode.empty()) {
    if (!createLua(luaCode)) {
      return;
    }
    state = StateReady;
    return;
  }

  luaCode.clear();
  std::string msg;
  msg += "getcode " VERSION;
  if (loadLuaCode(luaCode)) {
    msg += " " + TextUtils::itoa(luaCode.size());
    msg += " " + MD5(luaCode).hexdigest();
  }
  sendMessage(msg);
  debugf(1, "initial message = '%s'\n", msg.c_str());

  state = StateGetScript;
}


void HubLink::stateGetScript()
{
  if (!updateRecv() || !updateSend()) {
    return;
  }

  std::string code;
  if (!nextMessage(code)) {
    return;
  }

  if (code.empty()) {
    debugf(1, "received empty lua code\n");
  } else {
    debugf(1, "received %i bytes of lua code\n", (int)code.size());
    luaCode = code;
  }

  if (luaCode.empty()) {
    debugf(1, "empty luaCode\n");
    fail("empty luaCode");
    return;
  }

  if (!createLua(luaCode)) {
    return;
  }

  if (!code.empty()) {
    // save the new code (after it has been successfully used)
    saveLuaCode(code);
  }

  state = StateReady;
}


void HubLink::stateReady()
{
  updateLua();

  if (!updateRecv() || !updateSend()) {
    return;
  }

  std::string msg;
  while (nextMessage(msg)) {
    recvMessage(msg);
  }
}


//============================================================================//
//
//  network queue routines
//

bool HubLink::updateRecv()
{
  char buf[4096];
  const int bytes = read(sock, buf, sizeof(buf));
  if (bytes > 0) {
    debugf(4, "received %i bytes\n", bytes);
    recvTotal += bytes;
    recvQueue.push_back(std::string(buf, bytes));
  }
  else if (bytes == 0) {
    fail("disconnected");
    return false;
  }
  else if (bytes == -1) {
    if (errno != EAGAIN) {
      fail("recv error: ", errno);
      return false;
    }
  }
  else {
    debugf(1, "unknown recv state\n");
    return false;
  }
  return true;
}


bool HubLink::updateSend()
{
  while (sendTotal > 0) {
    const std::string& data = sendQueue.front();
    const int bytes = send(sock, data.c_str(), data.size(), 0);
    if (bytes > 0) {
      sendTotal -= bytes;
      if (bytes == (int)data.size()) {
        sendQueue.pop_front();
      } else {
        const std::string s = data.substr(bytes);
        sendQueue.pop_front();
        sendQueue.push_front(s);
      }
    }
    else if (bytes == 0) {
      fail("disconnected");
      return false;
    }
    else if (bytes == -1) {
      if (errno != EAGAIN) {
        fail("send error: ", errno);
        return false;
      }
    }
    else {
      debugf(1, "unknown send state\n");
      return false;
    }
  }
  return true;
}


bool HubLink::combineRecv(size_t minSize)
{
  if (recvTotal < minSize) {
    return false;
  }
  while (recvQueue.front().size() < minSize) {
    const std::string m0 = recvQueue.front(); recvQueue.pop_front();
    const std::string m1 = recvQueue.front(); recvQueue.pop_front();
    recvQueue.push_front(m0 + m1);
  }
  return true;
}


bool HubLink::nextMessage(std::string& msg)
{
  msg = "";

  const size_t lenSize = sizeof(uint32_t);
  if (!combineRecv(lenSize)) {
    return false;
  }

  uint32_t msgLen;
  nboUnpackUInt32((void*)recvQueue.front().c_str(), msgLen);
  const uint32_t totalLen = lenSize + msgLen;

  if (!combineRecv(totalLen)) {
    return false;
  }

  msg = recvQueue.front().substr(lenSize, msgLen);

  if (recvQueue.front().size() == totalLen) {
    recvQueue.pop_front();
  }
  else {
    const std::string s = recvQueue.front();
    recvQueue.pop_front();
    recvQueue.push_front(s.substr(totalLen));
  }

  recvTotal -= totalLen;

  return true;    
}


bool HubLink::sendMessage(const std::string& msg)
{
  const size_t lenSize = sizeof(uint32_t);
  char buf[lenSize];
  nboPackUInt32(buf, (uint32_t)msg.size());

  sendQueue.push_back(std::string(buf, lenSize) + msg);

  sendTotal += lenSize + msg.size();

  return updateSend();
}


//============================================================================//
//============================================================================//
//
//  Lua routines
//

static void PushNamedString(lua_State* L, const char* key,
                                          const char* value)
{
  lua_pushstring(L, key);
  lua_pushstring(L, value);
  lua_rawset(L, -3);
}


bool HubLink::createLua(const std::string& code)
{
  L = luaL_newstate();
  if (L == NULL) {
    fail("luaL_newstate() error");
    return false;
  }

  luaL_openlibs(L);

  if (!pushAnsiCodes()) {
    fail("pushAnsiCodes() error");
    return false;
  }
  if (!pushCallOuts()) {
    fail("pushCallOuts() error");
    return false;
  }
  if (!pushConstants()) {
    fail("pushConstants() error");
    return false;
  }

  lua_pushlightuserdata(L, this);
  lua_setfield(L, LUA_REGISTRYINDEX, ThisLabel);

  if (luaL_loadstring(L, code.c_str()) != 0) {
    std::string msg= "luaL_loadstring() error: ";
    msg += lua_tostring(L, -1);
    fail(msg);
    return false;
  }

  if (lua_pcall(L, 0, 0, 0) != 0) {
    std::string msg = "lua_pcall() error: ";
    msg += lua_tostring(L, -1);
    fail(msg);
    return false;
  }

  const ServerLink* srvLink = ServerLink::getServer();
  if ((srvLink != NULL) &&
      (srvLink->getState() == ServerLink::Okay)) {
    serverJoined(srvLink->getJoinServer(),
                 srvLink->getJoinPort(),
                 srvLink->getJoinCallsign());
  }

  return true;
}


bool HubLink::pushAnsiCodes()
{
  lua_pushvalue(L, LUA_GLOBALSINDEX);

  lua_pushliteral(L, "ANSI");
  lua_newtable(L);

  PushNamedString(L, "RESET",        ANSI_STR_RESET_FINAL);
  PushNamedString(L, "RESET_BRIGHT", ANSI_STR_RESET);
  PushNamedString(L, "BRIGHT",       ANSI_STR_BRIGHT);
  PushNamedString(L, "DIM",          ANSI_STR_DIM);
  PushNamedString(L, "UNDERLINE",    ANSI_STR_UNDERLINE);
  PushNamedString(L, "NO_UNDERLINE", ANSI_STR_NO_UNDERLINE);
  PushNamedString(L, "BLINK",        ANSI_STR_PULSATING);
  PushNamedString(L, "NO_BLINK",     ANSI_STR_NO_PULSATE);
  PushNamedString(L, "REVERSE",      ANSI_STR_REVERSE);
  PushNamedString(L, "NO_REVERSE",   ANSI_STR_NO_REVERSE);
  PushNamedString(L, "BLACK",   ANSI_STR_FG_BLACK);
  PushNamedString(L, "RED",     ANSI_STR_FG_RED);
  PushNamedString(L, "GREEN",   ANSI_STR_FG_GREEN);
  PushNamedString(L, "YELLOW",  ANSI_STR_FG_YELLOW);
  PushNamedString(L, "BLUE",    ANSI_STR_FG_BLUE);
  PushNamedString(L, "MAGENTA", ANSI_STR_FG_MAGENTA);
  PushNamedString(L, "CYAN",    ANSI_STR_FG_CYAN);
  PushNamedString(L, "WHITE",   ANSI_STR_FG_WHITE);
  PushNamedString(L, "ORANGE",  ANSI_STR_FG_ORANGE);
  lua_rawset(L, -3);

  lua_pop(L, 1); // pop _G

  return true;
}


//============================================================================//

bool HubLink::pushCallIn(const char* funcName, int inArgs)
{
  if (L == NULL) {
    return false;
  }
  if (!lua_checkstack(L, inArgs + 2)) {
    return false;
  }
  lua_getglobal(L, funcName);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    return false;
  }
  return true;
  
}


bool HubLink::runCallIn(int inArgs, int outArgs)
{
  if (lua_pcall(L, inArgs, outArgs, 0) != 0) {
    debugf(2, "error, %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    return false;
  }
  return true;
}


//============================================================================//
//
//  Lua Call-Ins
//

void HubLink::shutdown()
{
  if (!pushCallIn("Shutdown", 0)) {
    return;
  }
  runCallIn(0, 0);
}


void HubLink::updateLua()
{
  if (!pushCallIn("Update", 0)) {
    return;
  }
  runCallIn(0, 0);
}


void HubLink::recvCommand(const std::string& cmd)
{
  if (!pushCallIn("RecvCommand", 1)) {
    return;
  }

  lua_pushstdstring(L, cmd);

  runCallIn(1, 0);
}


void HubLink::recvMessage(const std::string& msg)
{
  debugf(2, "recvMessage(%i) = '%s'\n", (int)msg.size(),  msg.c_str());
  if (!pushCallIn("RecvMessage", 1)) {
    return;
  }

  lua_pushstdstring(L, msg);

  runCallIn(1, 0);
}


void HubLink::serverJoined(const std::string& location, int port,
                           const std::string& callsign)
{
  if (!pushCallIn("ServerJoined", 3)) {
    return;
  }

  lua_pushstdstring(L, location);
  lua_pushinteger(L,   port);
  lua_pushstdstring(L, callsign);

  runCallIn(3, 0);
}


void HubLink::serverParted()
{
  if (!pushCallIn("ServerParted", 0)) {
    return;
  }
  runCallIn(0, 0);
}


void HubLink::wordComplete(const std::string& line,
                              std::set<std::string>& matches)
{
  if (!pushCallIn("WordComplete", 1)) {
    return;
  }

  lua_pushstdstring(L, line);

  if (!runCallIn(1, 1)) {
    return;
  }

  const int table = lua_gettop(L);
  if (!lua_istable(L, table)) {
    lua_pop(L, 1);
    return;
  }

  for (int i = 1; lua_checkgeti(L, table, i++); lua_pop(L, 1)) {
    if (lua_israwstring(L, -1)) {
      matches.insert(lua_tostring(L, -1));
    }
  }
}


void HubLink::activeTabChanged()
{
  if (!pushCallIn("ActiveTabChanged", 0)) {
    return;
  }

  runCallIn(0, 0);
}


void HubLink::bzdbChange(const std::string& varName)
{
  if (!pushCallIn("BZDBChange", 1)) {
    return;
  }

  lua_pushstdstring(L, varName);

  runCallIn(1, 0);
}


void HubLink::startComposing()
{
  if (!pushCallIn("StartComposing", 0)) {
    return;
  }

  runCallIn(0, 0);
}


//============================================================================//
//
//  Lua Call-Outs
//

static inline HubLink* GetLink(lua_State* L)
{
  lua_getfield(L, LUA_REGISTRYINDEX, ThisLabel);
  if (!lua_isuserdata(L, -1)) {
    luaL_error(L, "Internal error -- missing '%s'", ThisLabel);
  }
  HubLink* link = (HubLink*)lua_touserdata(L, -1);
  lua_pop(L, 1);
  return link;
}


static int ParseTab(lua_State* L, int index)
{
  if (controlPanel == NULL) {
    return -1;
  }
  switch (lua_type(L, index)) {
    case LUA_TNUMBER: {
      return lua_tonumber(L, index);
    }
    case LUA_TSTRING: {
      return controlPanel->getTabID(lua_tostring(L, index));
    }
  }
  return -1;
}


#if defined(HAVE_UNISTD_H) && defined(HAVE_FCNTL_H)
static int ReadStdin(lua_State* L);
#endif


bool HubLink::pushCallOuts()
{
  lua_newtable(L);
  PUSH_LUA_CFUNC(L, Reload);
  PUSH_LUA_CFUNC(L, Disable);
  PUSH_LUA_CFUNC(L, SendMessage);
  PUSH_LUA_CFUNC(L, Print);
  PUSH_LUA_CFUNC(L, SetAlert);
  PUSH_LUA_CFUNC(L, AddTab);
  PUSH_LUA_CFUNC(L, RemoveTab);
  PUSH_LUA_CFUNC(L, ShiftTab);
  PUSH_LUA_CFUNC(L, ClearTab);
  PUSH_LUA_CFUNC(L, RenameTab);
  PUSH_LUA_CFUNC(L, GetTabCount);
  PUSH_LUA_CFUNC(L, GetTabLabel);
  PUSH_LUA_CFUNC(L, GetActiveTab);
  PUSH_LUA_CFUNC(L, SetActiveTab);
  PUSH_LUA_CFUNC(L, GetComposePrompt);
  PUSH_LUA_CFUNC(L, SetComposePrompt);
  PUSH_LUA_CFUNC(L, GetComposeString);
  PUSH_LUA_CFUNC(L, SetComposeString);
  PUSH_LUA_CFUNC(L, CalcMD5);
  PUSH_LUA_CFUNC(L, StripAnsiCodes);
  PUSH_LUA_CFUNC(L, SetBZDB);
  PUSH_LUA_CFUNC(L, GetBZDB);
  PUSH_LUA_CFUNC(L, GetCode);
  PUSH_LUA_CFUNC(L, GetTime);
  PUSH_LUA_CFUNC(L, GetVersion);
  PUSH_LUA_CFUNC(L, GetHubServer);
  PUSH_LUA_CFUNC(L, GetServerInfo);
#if defined(HAVE_UNISTD_H) && defined(HAVE_FCNTL_H)
  PUSH_LUA_CFUNC(L, ReadStdin);
#endif
  lua_setglobal(L, "bz");
  return true;
}


bool HubLink::pushConstants()
{
#define PUSH_LUA_STRING_NUMBER(s, n) \
  lua_pushstring(L, s); \
  lua_pushnumber(L, n); \
  lua_rawset(L, -3);

  lua_newtable(L);
  PUSH_LUA_STRING_NUMBER("ALLTABS", ControlPanel::MessageAllTabs);
  PUSH_LUA_STRING_NUMBER("CURRENT", ControlPanel::MessageCurrent);
  PUSH_LUA_STRING_NUMBER("ALL",     ControlPanel::MessageAll);
  PUSH_LUA_STRING_NUMBER("CHAT",    ControlPanel::MessageChat);
  PUSH_LUA_STRING_NUMBER("SERVER",  ControlPanel::MessageServer);
  PUSH_LUA_STRING_NUMBER("MISC",    ControlPanel::MessageMisc);
  PUSH_LUA_STRING_NUMBER("DEBUG",   ControlPanel::MessageDebug);
  lua_setglobal(L, "TABS");

#undef PUSH_LUA_STRING_NUMBER

  return true;
}


int HubLink::Reload(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  link->reloadHostPort = lua_israwstring(L, 1) ? lua_tostring(L, 1)
                                               : link->getHostPort();
  link->reloadLuaCode  = luaL_optstring(L, 2, "");
  return 0;
}


int HubLink::Disable(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  link->wantDisable = true;
  return 0;
}


int HubLink::AddTab(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  if (!controlPanel) {
    return 0;
  }
  const std::string label = luaL_checkstring(L, 1);
  const bool allSrc = !lua_isboolean(L, 2) || lua_toboolean(L, 2);
  const bool allDst = !lua_isboolean(L, 3) || lua_toboolean(L, 3);
  const bool success = controlPanel->addTab(label, allSrc, allDst);
  if (success) {
    link->tabs.insert(label);
  }
  lua_pushboolean(L, success);
  return 1;
}


int HubLink::RemoveTab(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  if (!controlPanel) {
    return 0;
  }
  const std::string label = luaL_checkstring(L, 1);
  link->tabs.erase(label);
  lua_pushboolean(L, controlPanel->removeTab(label));
  return 1;
}


int HubLink::ShiftTab(lua_State* L)
{
  if (!controlPanel) {
    return 0;
  }
  const int tabID = ParseTab(L, 1);
  const int distance  = luaL_checkint(L, 2);
  lua_pushboolean(L, controlPanel->shiftTab(tabID, distance));
  return 1;
}


int HubLink::ClearTab(lua_State* L)
{
  if (!controlPanel) {
    return 0;
  }
  const int tabID = ParseTab(L, 1);
  lua_pushboolean(L, controlPanel->clearTab(tabID));
  return 1;
}


int HubLink::RenameTab(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  if (!controlPanel) {
    return 0;
  }
  const std::string oldLabel = luaL_checkstring(L, 1);
  const std::string newLabel = luaL_checkstring(L, 2);
  const bool success = controlPanel->renameTab(oldLabel, newLabel);
  if (success) {
    link->tabs.erase(oldLabel);
    link->tabs.insert(newLabel);
  }
  lua_pushboolean(L, success);
  return 1;
}


int HubLink::GetTabCount(lua_State* L)
{
  if (!controlPanel) {
    return 0;
  }
  lua_pushinteger(L, controlPanel->getTabCount());
  return 1;
}


int HubLink::GetTabLabel(lua_State* L)
{
  if (!controlPanel) {
    return 0;
  }
  const int tabID = luaL_checkint(L, 1);
  const std::string& label = controlPanel->getTabLabel(tabID);
  if (label.empty()) {
    return 0;
  }
  lua_pushstdstring(L, label);
  return 1;
}


int HubLink::GetActiveTab(lua_State* L)
{
  if (!controlPanel) {
    return 0;
  }
  const int tabID = controlPanel->getActiveTab();
  if (tabID < 0) {
    return 0;
  }
  const std::string& label = controlPanel->getTabLabel(tabID);
  if (label.empty()) {
    return 0;
  }
  lua_pushstdstring(L, label);
  return 1;
}


int HubLink::GetComposePrompt(lua_State* L)
{
  if (hud == NULL) {
    return 0;
  }
  HUDuiDefaultKey* defKey = HUDui::getDefaultKey();
  if (!defKey || !dynamic_cast<HubComposeKey*>(defKey)) {
    return 0;
  }
  if (!hud->getComposing()) {
    return 0;
  }
  lua_pushstdstring(L, hud->getComposePrompt());
  return 1;
}


int HubLink::SetComposePrompt(lua_State* L)
{
  if (hud == NULL) {
    return 0;
  }
  HUDuiDefaultKey* defKey = HUDui::getDefaultKey();
  if (!defKey || !dynamic_cast<HubComposeKey*>(defKey)) {
    return 0;
  }
  if (!hud->getComposing()) {
    return 0;
  }
  const std::string prompt = luaL_checkstring(L, 1);
  hud->setComposing(prompt);
  lua_pushboolean(L, true);
  return 1;
}


int HubLink::GetComposeString(lua_State* L)
{
  if (hud == NULL) {
    return 0;
  }
  HUDuiDefaultKey* defKey = HUDui::getDefaultKey();
  if (!defKey || !dynamic_cast<HubComposeKey*>(defKey)) {
    return 0;
  }
  if (!hud->getComposing()) {
    return 0;
  }
  lua_pushstdstring(L, hud->getComposeString());
  return 1;
}


int HubLink::SetComposeString(lua_State* L)
{
  if (hud == NULL) {
    return 0;
  }
  HUDuiDefaultKey* defKey = HUDui::getDefaultKey();
  if (!defKey || !dynamic_cast<HubComposeKey*>(defKey)) {
    return 0;
  }
  if (!hud->getComposing()) {
    return 0;
  }
  const std::string prompt = luaL_checkstring(L, 1);
  hud->setComposeString(prompt);
  lua_pushboolean(L, true);
  return 1;
}


int HubLink::SetActiveTab(lua_State* L)
{
  if (!controlPanel) {
    return 0;
  }
  const int tabID = ParseTab(L, 1);
  lua_pushboolean(L, controlPanel->setActiveTab(tabID));
  return 1;
}


int HubLink::SendMessage(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  const std::string msg = luaL_checkstring(L, 1);
  link->sendMessage(msg);
  return 0;
}


int HubLink::Print(lua_State* L)
{
  const std::string msg = luaL_checkstring(L, 1);

  switch (lua_type(L, 2)) {
    case LUA_TNUMBER: {
      controlPanel->addMessage(msg, lua_tointeger(L, 2));
      break;
    }
    case LUA_TSTRING: {
      controlPanel->addMessage(msg, lua_tostring(L, 2));
      break;
    }
    default: {
      controlPanel->addMessage(msg);
      break;
    }
  }
  lua_pushboolean(L, true);
  return 1;
}


int HubLink::SetAlert(lua_State* L)
{
  const std::string msg = luaL_checkstring(L, 1);
  const int   priority  = luaL_optint(L, 2, 0);
  const float duration  = luaL_optfloat(L, 3, 3.0f);
  const bool  warning   = lua_isboolean(L, 4) && lua_toboolean(L, 4);

  hud->setAlert(priority, msg.c_str(), duration, warning);
  return 0;
}


int HubLink::CalcMD5(lua_State* L)
{
  const std::string text = luaL_checkstring(L, 1);
  lua_pushstdstring(L, MD5(text).hexdigest());
  return 1;
}


int HubLink::StripAnsiCodes(lua_State* L)
{
  const std::string text = luaL_checkstring(L, 1);
  lua_pushstdstring(L, stripAnsiCodes(text));
  return 1;
}


int HubLink::SetBZDB(lua_State* L)
{
  const std::string key    = luaL_checkstring(L, 1);
  const std::string value  = luaL_checkstring(L, 2);
  const bool usePersistent = lua_isboolean(L, 3);
  const bool isPersistent  = usePersistent && lua_toboolean(L, 3);

  if (BZDB.isSet(key) &&
      (BZDB.getPermission(key) != StateDatabase::Client)) {
    lua_pushboolean(L, false);
    return 1;
  }

  BZDB.set(key, value);

  if (usePersistent) {
    BZDB.setPersistent(key, isPersistent);
  }

  lua_pushboolean(L, true);
  return 1;
}


int HubLink::GetBZDB(lua_State* L)
{
  const std::string key = luaL_checkstring(L, 1);
  if (!BZDB.isSet(key)) {
    return 0;
  }
  lua_pushstdstring(L, BZDB.get(key));
  return 1;
}


int HubLink::GetCode(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  lua_pushstdstring(L, link->getLuaCode());
  return 1;
}


int HubLink::GetTime(lua_State* L)
{
  lua_pushdouble(L, TimeKeeper::getCurrent().getSeconds());
  return 1;
}


int HubLink::GetVersion(lua_State* L)
{
  lua_pushstring(L, VERSION);
  lua_pushstring(L, getAppVersion());
  return 2;
}


int HubLink::GetHubServer(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  lua_pushstdstring(L, link->getHostPort());
  return 1;
}


int HubLink::GetServerInfo(lua_State* L)
{
  const ServerLink* srvLink = ServerLink::getServer();
  if ((srvLink == NULL) ||
      (srvLink->getState() != ServerLink::Okay)) {
    return 0;
  }

  lua_pushstdstring(L, srvLink->getJoinServer());
  lua_pushinteger(L,   srvLink->getJoinPort());
  lua_pushstdstring(L, srvLink->getJoinCallsign());

  return 3;
}


//============================================================================//
//============================================================================//

// whacky bit of dev'ing fun
#if defined(HAVE_UNISTD_H) && defined(HAVE_FCNTL_H)
  #include <unistd.h>
  #include <fcntl.h>
  static int ReadStdin(lua_State* L)
  {
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    char buf[4096];
    const int r = read(STDIN_FILENO, buf, sizeof(buf));
    if (r <= 0) {
      return 0;
    }
    lua_pushlstring(L, buf, r);
    fcntl(STDIN_FILENO, F_SETFL, 0);
    return 1;
  }
#endif


//============================================================================//
//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
