/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "BZAdminUI.h"
#if (defined(HAVE_CURSES_H) || defined (HAVE_NCURSES_H))
#include "CursesUI.h"
#endif
#include "OptionParser.h"
#include "ServerLink.h"
#include "StdBothUI.h"
#include "StdInUI.h"
#include "StdOutUI.h"
#include "UIMap.h"

// causes persistent rebuilding to obtain build versioning
#include "version.h"

using namespace std;


// function prototypes
/** Checks for new packets from the server, ignores them or stores a
    text message in str. Tells ui about new or removed players. Returns
    false if no interesting packets have arrived. */
bool getServerString(ServerLink& sLink, string& str, BZAdminUI& ui);

/** Sends the message msg to the server. */
void sendMessage(ServerLink& sLink, const string& msg, PlayerId target);

/** Formats an incoming message. */
string formatMessage(const string& msg, PlayerId src,
		     PlayerId dst, TeamColor dstTeam, PlayerId me);


// some global variables
map<PlayerId, string> players;
TeamColor myTeam(ObserverTeam);
struct CLOptions {
  CLOptions() : ui("curses"), showHelp(false) { }
  string ui;
  bool showHelp;
} clOptions;


// Here we go.
int main(int argc, char** argv) {

  // no curses, use stdboth instead
  const UIMap::map_t& interfaces(UIMap::getInstance().getMap());
  if (interfaces.find("curses") == interfaces.end())
    clOptions.ui = "stdboth";
  
  // build a usage string with all interfaces
  UIMap::map_t::const_iterator uiIter;
  string uiUsage;
  for (uiIter = interfaces.begin(); uiIter != interfaces.end(); ++uiIter)
    uiUsage += uiIter->first + '|';
  uiUsage = string("[-ui ") + uiUsage.substr(0, uiUsage.size() - 1) + ']';
  
  // register and parse command line arguments
  OptionParser op;
  op.registerVariable("ui", clOptions.ui, uiUsage,
		      "choose a user interface");
  op.registerVariable("help", clOptions.showHelp, "[-help]",
		      "print this help message");
  if (!op.parse(argc, argv)) {
    cerr<<op.getError()<<endl;
    op.printUsage(cout, argv[0]);
    cout<<"CALLSIGN@HOST[:PORT] [COMMAND] [COMMAND] ..."<<endl;
    return 1;
  }
  if (clOptions.showHelp) {
    cout<<"bzadmin "<<getAppVersion()<<endl;
    op.printUsage(cout, argv[0]);
    cout<<"CALLSIGN@HOST[:PORT] [COMMAND] [COMMAND] ..."<<endl<<endl;
    op.printHelp(cout);
    return 0;
  }
  
  // check that the ui is valid
  uiIter = UIMap::getInstance().getMap().find(clOptions.ui);
  if (uiIter == UIMap::getInstance().getMap().end()) {
    cerr<<"There is no interface called \""<<clOptions.ui<<"\"."<<endl;
    return 1;
  }
  
  // check that we have callsign and host in the right format and extract them
  if (op.getParameters().size() == 0) {
    cerr<<"You have to specify callsign@host."<<endl;
    return 1;
  }
  const string& namehost = op.getParameters()[0];
  int atPos = namehost.find('@');
  if (atPos == -1) {
    cerr<<"You have to specify callsign@host."<<endl;
    return 1;
  }
  string name = namehost.substr(0, atPos);
  string host = namehost.substr(atPos + 1);
  int port = ServerPort;
  int cPos = host.find(':');
  if (cPos != -1) {
    port = atoi(host.substr(cPos + 1).c_str());
    host = host.substr(0, cPos);
  }

  // connect to the server
  Address a(host);
  ServerLink sLink(a, port);
  if (sLink.getState() != ServerLink::Okay) {
    cerr<<"Could not connect to "<<host<<':'<<port<<'.'<<endl;
    return 0;
  }
  sLink.sendEnter(TankPlayer, myTeam, name.c_str(), "");
  if (sLink.getState() != ServerLink::Okay) {
    cerr<<"Rejected."<<endl;
    return 0;
  }

  // if we got commands as arguments, send them and exit
  if (op.getParameters().size() > 1) {
    for (unsigned int i = 1; i < op.getParameters().size(); ++i)
      sendMessage(sLink, op.getParameters()[i], AllPlayers);
    return 0;
  }

  // choose UI
  BZAdminUI*  ui = uiIter->second(players, sLink.getId());

  // main loop
  string str;
  PlayerId me = sLink.getId();
  try {
    while (true) {
      while (getServerString(sLink, str, *ui))
	ui->outputMessage(str);
      if (ui->checkCommand(str)) {
	if (str == "/quit")
	  break;
	sendMessage(sLink, str, ui->getTarget());
	// private messages to other players aren't sent back to us, print here
	if (players.count(ui->getTarget()))
	  ui->outputMessage(formatMessage(str, me, 
					  ui->getTarget(), NoTeam, me));
      }
    }
    
    // we need to know that the server has processed all our messages
    // send a private message to ourself and wait for it to come back
    // this assumes that the order of messages isn't changed along the way
    if (sLink.getState() == ServerLink::Okay) {
      sendMessage(sLink, "bzadminping", me);
      string expected = formatMessage("bzadminping", me, me, NoTeam, me);
      do {
	getServerString(sLink, str, *ui);
      } while (str != expected);
    }
  }
  catch (const string& error) {
    ui->outputMessage(string("--- ERROR: ") + error);
  }
  delete ui;

  return 0;
}


bool getServerString(ServerLink& sLink, string& str, BZAdminUI& ui) {
  uint16_t code, len;
  char inbuf[MaxPacketLen];
  int e;
  std::string dstName, srcName;
  str = "";
  
  /* read until we have a package that we want, or until there are no more
     packages for 100 ms */
  while ((e = sLink.read(code, len, inbuf, 100)) == 1) {
    void* vbuf = inbuf;
    PlayerId p;
    switch (code) {

    case MsgAddPlayer:

      vbuf = nboUnpackUByte(vbuf, p);

      uint16_t team, type, wins, losses, tks;
      char callsign[CallSignLen];
      char email[EmailLen];
      vbuf = nboUnpackUShort(vbuf, type);
      vbuf = nboUnpackUShort(vbuf, team);
      vbuf = nboUnpackUShort(vbuf, wins);
      vbuf = nboUnpackUShort(vbuf, losses);
      vbuf = nboUnpackUShort(vbuf, tks);
      vbuf = nboUnpackString(vbuf, callsign, CallSignLen);
      vbuf = nboUnpackString(vbuf, email, EmailLen);
      players[p] = callsign;
      if (p != sLink.getId())
	ui.addedPlayer(p);
      str = str + "*** '" + callsign + "' joined the game.";
      return true;

    case MsgRemovePlayer:
      vbuf = nboUnpackUByte(vbuf, p);
      str = str + "*** '" + players[p] + "' left the game.";
      ui.removingPlayer(p);
      players.erase(p);
      return true;

    case MsgSuperKill:
      throw string("Server forced a disconnect");

    case MsgMessage:

      // unpack the message header
      PlayerId src;
      PlayerId dst;
      PlayerId me = sLink.getId();
      vbuf = nboUnpackUByte(vbuf, src);
      vbuf = nboUnpackUByte(vbuf, dst);

      // is the message for me?
      TeamColor dstTeam = (dst >= 244 && dst <= 250 ?
			   TeamColor(250 - dst) : NoTeam);
      if (dst == AllPlayers || src == me || dst == me || dstTeam == myTeam) {
	str = (char*)vbuf;
	if (str == "CLIENTQUERY") {
	  sendMessage(sLink, string("bzadmin ") + getAppVersion(), src);
	  ui.outputMessage("    [Sent versioninfo per request]");
	}
	else {
	  str = formatMessage((char*)vbuf, src, dst, dstTeam, me);
	  return true;
	}
      }
    }
  }
  
  if (e == -1) {
    throw string("Server communication error");
  }
  
  return false;
}


void sendMessage(ServerLink& sLink, const string& msg, PlayerId target) {
  char buffer[MessageLen];
  char buffer2[1 + MessageLen];
  void* buf = buffer2;

  buf = nboPackUByte(buf, target);
  memset(buffer, 0, MessageLen);
  strncpy(buffer, msg.c_str(), MessageLen);
  nboPackString(buffer2 + 1, buffer, MessageLen);
  sLink.send(MsgMessage, sizeof(buffer2), buffer2);
}


string formatMessage(const string& msg, PlayerId src,
		     PlayerId dst, TeamColor dstTeam, PlayerId me) {
  string formatted = "    ";

  // get sender and receiver
  const string srcName = (src == ServerPlayer ? "SERVER" :
			  (players.count(src) ? players[src] : "(UNKNOWN)"));
  const string dstName = (players.count(dst) ? players[dst] : "(UNKNOWN)");

  // direct message to or from me
  if (dst == me || players.count(dst)) {
    if (!(src == me && dst == me)) {
      formatted += "[";
      if (src == me) {
	formatted += "->";
	formatted += dstName;
      }
      else {
	formatted += srcName;
	formatted += "->";
      }
      formatted += "] ";
    }
    formatted += msg;
  }

  // public or team message
  else {
    if (dstTeam != NoTeam)
      formatted += "[Team] ";
    formatted += srcName;
    formatted += ": ";
    formatted += msg;
  }

  return formatted;
}
