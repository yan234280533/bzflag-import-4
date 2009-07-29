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

// interface header
#include "Constants.h"

// system headers
#include <string>

// common headers
#include "bzfsAPI.h"
#include "Obstacle.h"

// local headers
#include "LuaHeader.h"


static bool PushGames(lua_State* L);
static bool PushTeams(lua_State* L);
static bool PushShots(lua_State* L);
static bool PushFlagQualities(lua_State* L);
static bool PushPlayers(lua_State* L);
static bool PushPlayerStates(lua_State* L);
static bool PushObstacles(lua_State* L);
static bool PushPermissions(lua_State* L);


//============================================================================//
//============================================================================//

bool Constants::PushEntries(lua_State* L)
{
  PushGames(L);
  PushTeams(L);
  PushShots(L);
  PushFlagQualities(L);
  PushPlayers(L);
  PushPlayerStates(L);
  PushObstacles(L);
  PushPermissions(L);

  return true;
}


//============================================================================//
//============================================================================//

static void PushDualPair(lua_State* L, const char* name, int code)
{
  lua_pushstring(L, name);
  lua_pushinteger(L, code);
  lua_rawset(L, -3);
  lua_pushinteger(L, code);
  lua_pushstring(L, name);
  lua_rawset(L, -3);
}


//============================================================================//
//============================================================================//

static bool PushGames(lua_State* L)
{
  lua_pushliteral(L, "GAME");
  lua_newtable(L);

  PushDualPair(L, "FFA",    eTeamFFAGame);
  PushDualPair(L, "CTF",    eClassicCTFGame);
  PushDualPair(L, "RABBIT", eRabbitGame);
  PushDualPair(L, "OPEN",   eOpenFFAGame);

  lua_rawset(L, -3);

  return true;
}


static bool PushTeams(lua_State* L)
{
  lua_pushliteral(L, "TEAM");
  lua_newtable(L);

  PushDualPair(L, "AUTO",     eAutomaticTeam);
  PushDualPair(L, "NONE",     eNoTeam);
  PushDualPair(L, "ROGUE",    eRogueTeam);
  PushDualPair(L, "RED",      eRedTeam);
  PushDualPair(L, "GREEN",    eGreenTeam);
  PushDualPair(L, "BLUE",     eBlueTeam);
  PushDualPair(L, "PURPLE",   ePurpleTeam);
  PushDualPair(L, "RABBIT",   eRabbitTeam);
  PushDualPair(L, "HUNTER",   eHunterTeam);
  PushDualPair(L, "OBSERVER", eObservers);
  PushDualPair(L, "ADMIN",    eAdministrators);

  lua_rawset(L, -3);

  return true;
}


static bool PushShots(lua_State* L)
{
  lua_pushliteral(L, "SHOT");
  lua_newtable(L);

  PushDualPair(L, "NO",  eNoShot);
  PushDualPair(L, "STD", eStandardShot);
  PushDualPair(L, "GM",  eGMShot);
  PushDualPair(L, "L",   eLaserShot);
  PushDualPair(L, "TH",  eThiefShot);
  PushDualPair(L, "SH",  eSuperShot);
  PushDualPair(L, "PZ",  ePhantomShot);
  PushDualPair(L, "SW",  eShockWaveShot);
  PushDualPair(L, "R",   eRicoShot);
  PushDualPair(L, "MG",  eMachineGunShot);
  PushDualPair(L, "IB",  eInvisibleShot);
  PushDualPair(L, "CL",  eCloakedShot);
  PushDualPair(L, "F",   eRapidFireShot);

  lua_rawset(L, -3);

  return true;
}


static bool PushObstacles(lua_State* L)
{
  lua_pushliteral(L, "OBSTACLE");
  lua_newtable(L);

  PushDualPair(L, "WALL",    wallType);
  PushDualPair(L, "BOX",     boxType);
  PushDualPair(L, "PYRAMID", pyrType);
  PushDualPair(L, "BASE",    baseType);
  PushDualPair(L, "TELE",    teleType);
  PushDualPair(L, "MESH",    meshType);
  PushDualPair(L, "ARC",     arcType);
  PushDualPair(L, "CONE",    coneType);
  PushDualPair(L, "SPHERE",  sphereType);
  PushDualPair(L, "FACE",    faceType);

  lua_rawset(L, -3);

  return true;
}


static bool PushFlagQualities(lua_State* L)
{
  lua_pushliteral(L, "FLAGQUAL");
  lua_newtable(L);

  PushDualPair(L, "GOOD", eGoodFlag);
  PushDualPair(L, "BAD",  eBadFlag);

  lua_rawset(L, -3);

  return true;
}


static bool PushPlayers(lua_State* L)
{
  lua_pushliteral(L, "PLAYER");
  lua_newtable(L);

  PushDualPair(L, "SERVER", BZ_SERVER);
  PushDualPair(L, "ALL",    BZ_ALLUSERS);
  PushDualPair(L, "NULL",   BZ_NULLUSER);

  lua_rawset(L, -3);

  return true;
}


static bool PushPlayerStates(lua_State* L)
{
  lua_pushliteral(L, "STATUS");
  lua_newtable(L);

  PushDualPair(L, "DEAD",        eDead);
  PushDualPair(L, "ALIVE",       eAlive);
  PushDualPair(L, "PAUSED",      ePaused);
  PushDualPair(L, "EXPLODING",   eExploding);
  PushDualPair(L, "TELEPORTING", eTeleporting);
  PushDualPair(L, "INBUILDING",  eInBuilding);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//

static bool PushPermissions(lua_State* L)
{
  lua_pushliteral(L, "PERM");
  lua_newtable(L);

#define ADD_PERM(x)                  \
  lua_pushliteral(L, #x);            \
  lua_pushliteral(L, bz_perm_ ## x); \
  lua_rawset(L, -3);

  ADD_PERM(actionMessage);
  ADD_PERM(adminMessageReceive);
  ADD_PERM(adminMessageSend);
  ADD_PERM(antiban);
  ADD_PERM(antikick);
  ADD_PERM(antikill);
  ADD_PERM(antipoll);
  ADD_PERM(antipollban);
  ADD_PERM(antipollkick);
  ADD_PERM(antipollkill);
  ADD_PERM(ban);
  ADD_PERM(banlist);
  ADD_PERM(countdown);
  ADD_PERM(date);
  ADD_PERM(endGame);
  ADD_PERM(flagHistory);
  ADD_PERM(flagMod);
  ADD_PERM(hideAdmin);
  ADD_PERM(idleStats);
  ADD_PERM(info);
  ADD_PERM(kick);
  ADD_PERM(kill);
  ADD_PERM(lagStats);
  ADD_PERM(lagwarn);
  ADD_PERM(listPlugins);
  ADD_PERM(listPerms);
  ADD_PERM(luaServer);
  ADD_PERM(masterBan);
  ADD_PERM(mute);
  ADD_PERM(playerList);
  ADD_PERM(poll);
  ADD_PERM(pollBan);
  ADD_PERM(pollKick);
  ADD_PERM(pollKill);
  ADD_PERM(pollSet);
  ADD_PERM(pollFlagReset);
  ADD_PERM(privateMessage);
  ADD_PERM(record);
  ADD_PERM(rejoin);
  ADD_PERM(removePerms);
  ADD_PERM(replay);
  ADD_PERM(say);
  ADD_PERM(sendHelp);
  ADD_PERM(setAll);
  ADD_PERM(setPerms);
  ADD_PERM(setVar);
  ADD_PERM(showOthers);
  ADD_PERM(shortBan);
  ADD_PERM(shutdownServer);
  ADD_PERM(spawn);
  ADD_PERM(superKill);
  ADD_PERM(talk);
  ADD_PERM(unban);
  ADD_PERM(unmute);
  ADD_PERM(veto);
  ADD_PERM(viewReports);
  ADD_PERM(vote);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
