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
#include "LuaLoader.h"

// system headers
#include <string>
#include <stdlib.h>

// local headers
#include "LuaHeader.h"


static const char* ThisLabel = "this";


static bool PushCallOuts(lua_State* L);

static int Execute(lua_State* L);

static int DoNothing(lua_State* L);
static int Fire(lua_State* L);
static int Resume(lua_State* L);
static int Scan(lua_State* L);
static int Stop(lua_State* L);
static int Ahead(lua_State* L);
static int Back(lua_State* L);
static int TurnLeft(lua_State* L);
static int TurnRight(lua_State* L);

static int SetAhead(lua_State* L);
static int SetFire(lua_State* L);
static int SetTurnRate(lua_State* L);
static int SetMaxVelocity(lua_State* L);
static int SetResume(lua_State* L);
static int SetStop(lua_State* L);
static int SetTurnLeft(lua_State* L);

static int GetTime(lua_State* L);
static int GetBattleFieldSize(lua_State* L);
static int GetGunCoolingRate(lua_State* L);
static int GetGunHeat(lua_State* L);
static int GetHeading(lua_State* L);
static int GetHeight(lua_State* L);
static int GetLength(lua_State* L);
static int GetName(lua_State* L);
static int GetTime(lua_State* L);
static int GetWidth(lua_State* L);
static int GetVelocity(lua_State* L);
static int GetX(lua_State* L);
static int GetY(lua_State* L);
static int GetZ(lua_State* L);
static int GetDistanceRemaining(lua_State* L);
static int GetTurnRemaining(lua_State* L);

#if defined(HAVE_UNISTD_H) && defined(HAVE_FCNTL_H)
static int ReadStdin(lua_State* L);
#endif


//============================================================================//
//============================================================================//

class LuaRobot : public BZAdvancedRobot {
  public:
    LuaRobot(const std::string& filename);
    ~LuaRobot();

    void run();

    void onBattleEnded(const BattleEndedEvent&);
    void onBulletHit(const BulletHitEvent&);
    void onBulletMissed(const BulletMissedEvent&);
    void onDeath(const DeathEvent&);
    void onHitByBullet(const HitByBulletEvent&);
    void onHitWall(const HitWallEvent&);
    void onRobotDeath(const RobotDeathEvent&);
    void onScannedRobot(const ScannedRobotEvent&);
    void onSpawn(const SpawnEvent&);
    void onStatus(const StatusEvent&);
    void onWin(const WinEvent&);

  private:
    bool PushFunction(const char* funcName, int inArgs);
    bool ExecFunction(int inArgs, int outArgs);

  private:
    lua_State* L;
};


//============================================================================//
//============================================================================//
//
//  LuaLoader
//

LuaLoader::LuaLoader()
{
  _running = true;
  error = "";
}


LuaLoader::~LuaLoader()
{
}


bool LuaLoader::load(std::string filename)
{
  scriptFile = filename;
  _loaded = true;
  return true;
}


BZRobot* LuaLoader::create()
{
  return new LuaRobot(scriptFile);
}


void LuaLoader::destroy(BZRobot* instance)
{
  delete instance;
}


//============================================================================//
//============================================================================//
//
//  LuaRobot
//

LuaRobot::LuaRobot(const std::string& filename)
{
  L = luaL_newstate();

  luaL_openlibs(L);

  lua_pushlightuserdata(L, (void*)this);
  lua_setfield(L, LUA_REGISTRYINDEX, ThisLabel);

  if (!PushCallOuts(L)) {
    printf("LuaRobot: failed to push call-outs\n");
    lua_close(L);
    L = NULL;
    return;
  }

  // compile the code
  if (luaL_loadfile(L, filename.c_str()) != 0) {
    printf("LuaRobot: failed to load '%s'\n", filename.c_str());
    lua_close(L);
    L = NULL;
    return;
  }

  // execute the code
  if (lua_pcall(L, 0, 0, 0) != 0) {
    printf("LuaRobot: failed to execute, %s\n", lua_tostring(L, -1));
    lua_close(L);
    L = NULL;
    return;
  }
}


LuaRobot::~LuaRobot()
{
  if (L != NULL) {
    lua_close(L);
  }
}


//============================================================================//
//============================================================================//
//
//  Call-ins
//

bool LuaRobot::PushFunction(const char* funcName, int inArgs)
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


bool LuaRobot::ExecFunction(int inArgs, int outArgs)
{
  if (lua_pcall(L, inArgs, outArgs, 0) != 0) {
    printf("LuaRobot: error, %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    return false;
  }
  return true;
}


static void PushBullet(lua_State* L, const Bullet* bullet)
{
  lua_createtable(L, 0, 8);
  if (bullet == NULL) {
    return;
  }
  lua_pushstdstring(L, bullet->getName());   lua_setfield(L, -2, "owner");
  lua_pushstdstring(L, bullet->getVictim()); lua_setfield(L, -2, "victim");
  lua_pushdouble(L, bullet->getVelocity());  lua_setfield(L, -2, "velocity");
  lua_pushdouble(L, bullet->getHeading());   lua_setfield(L, -2, "heading");
  lua_pushdouble(L, bullet->getX());         lua_setfield(L, -2, "x");
  lua_pushdouble(L, bullet->getY());         lua_setfield(L, -2, "y");
  lua_pushdouble(L, bullet->getZ());         lua_setfield(L, -2, "z");
  lua_pushboolean(L, bullet->isActive());    lua_setfield(L, -2, "active");
}


//============================================================================//

void LuaRobot::run()
{
  if (!PushFunction("Run", 0)) {
    printf("LuaRobot: missing Run() function\n");
    return;
  }
  ExecFunction(0, 0);
}


void LuaRobot::onBattleEnded(const BattleEndedEvent& event)
{
  if (!PushFunction("BattleEnded", 3)) {
    return;
  }
  lua_pushdouble(L, event.getTime());
  lua_pushinteger(L, event.getPriority());
  lua_pushboolean(L, event.isAborted());
  ExecFunction(3, 0);
}


void LuaRobot::onBulletHit(const BulletHitEvent& event)
{
  if (!PushFunction("BulletHit", 4)) {
    return;
  }
  lua_pushdouble(L, event.getTime());
  lua_pushinteger(L, event.getPriority());
  lua_pushstdstring(L, event.getName());
  PushBullet(L, event.getBullet());
  ExecFunction(4, 0);
}


void LuaRobot::onBulletMissed(const BulletMissedEvent& event)
{
  if (!PushFunction("BulletMissed", 3)) {
    return;
  }
  lua_pushdouble(L, event.getTime());
  lua_pushinteger(L, event.getPriority());
  PushBullet(L, event.getBullet());
  ExecFunction(3, 0);
}


void LuaRobot::onDeath(const DeathEvent& event)
{
  if (!PushFunction("Death", 2)) {
    return;
  }
  lua_pushdouble(L, event.getTime());
  lua_pushinteger(L, event.getPriority());
  ExecFunction(2, 0);
}


void LuaRobot::onHitByBullet(const HitByBulletEvent& event)
{
  if (!PushFunction("HitByBullet", 4)) {
    return;
  }
  lua_pushdouble(L, event.getTime());
  lua_pushinteger(L, event.getPriority());
  lua_pushdouble(L, event.getBearing());
  PushBullet(L, event.getBullet());
  ExecFunction(4, 0);
}


void LuaRobot::onHitWall(const HitWallEvent& event)
{
  if (!PushFunction("HitWall", 3)) {
    return;
  }
  lua_pushdouble(L, event.getTime());
  lua_pushinteger(L, event.getPriority());
  lua_pushdouble(L, event.getBearing());
  ExecFunction(3, 0);
}


void LuaRobot::onRobotDeath(const RobotDeathEvent& event)
{
  if (!PushFunction("RobotDeath", 2)) {
    return;
  }
  lua_pushdouble(L, event.getTime());
  lua_pushinteger(L, event.getPriority());
  ExecFunction(2, 0);
}


void LuaRobot::onScannedRobot(const ScannedRobotEvent& event)
{
  if (!PushFunction("ScannedRobot", 3)) {
    return;
  }
  lua_pushdouble(L, event.getTime());
  lua_pushinteger(L, event.getPriority());

  lua_createtable(L, 0, 8);
  lua_pushstdstring(L, event.getName());  lua_setfield(L, -2, "name");
  lua_pushdouble(L, event.getBearing());  lua_setfield(L, -2, "bearing");
  lua_pushdouble(L, event.getDistance()); lua_setfield(L, -2, "distance");
  lua_pushdouble(L, event.getX());        lua_setfield(L, -2, "x");
  lua_pushdouble(L, event.getY());        lua_setfield(L, -2, "y");
  lua_pushdouble(L, event.getZ());        lua_setfield(L, -2, "z");
  lua_pushdouble(L, event.getHeading());  lua_setfield(L, -2, "heading");
  lua_pushdouble(L, event.getVelocity()); lua_setfield(L, -2, "velocity");

  ExecFunction(3, 0);
}


void LuaRobot::onSpawn(const SpawnEvent& event)
{
  if (!PushFunction("Spawn", 2)) {
    return;
  }
  lua_pushdouble(L, event.getTime());
  lua_pushinteger(L, event.getPriority());
  ExecFunction(2, 0);
}


void LuaRobot::onStatus(const StatusEvent& event)
{
  if (!PushFunction("Status", 2)) {
    return;
  }
  lua_pushdouble(L, event.getTime());
  lua_pushinteger(L, event.getPriority());
  ExecFunction(2, 0);
}


void LuaRobot::onWin(const WinEvent& event)
{
  if (!PushFunction("Win", 2)) {
    return;
  }
  lua_pushdouble(L, event.getTime());
  lua_pushinteger(L, event.getPriority());
  ExecFunction(2, 0);
}


//============================================================================//
//============================================================================//
//
//  Call-outs
//

static bool PushCallOuts(lua_State* L)
{
  lua_newtable(L);

  PUSH_LUA_CFUNC(L, Execute);

  PUSH_LUA_CFUNC(L, DoNothing);
  PUSH_LUA_CFUNC(L, Fire);
  PUSH_LUA_CFUNC(L, Resume);
  PUSH_LUA_CFUNC(L, Scan);
  PUSH_LUA_CFUNC(L, Stop);
  PUSH_LUA_CFUNC(L, Ahead);
  PUSH_LUA_CFUNC(L, Back);
  PUSH_LUA_CFUNC(L, TurnLeft);
  PUSH_LUA_CFUNC(L, TurnRight);

  PUSH_LUA_CFUNC(L, SetAhead);
  PUSH_LUA_CFUNC(L, SetFire);
  PUSH_LUA_CFUNC(L, SetTurnRate);
  PUSH_LUA_CFUNC(L, SetMaxVelocity);
  PUSH_LUA_CFUNC(L, SetResume);
  PUSH_LUA_CFUNC(L, SetStop);
  PUSH_LUA_CFUNC(L, SetTurnLeft);

  PUSH_LUA_CFUNC(L, GetTime);
  PUSH_LUA_CFUNC(L, GetBattleFieldSize);
  PUSH_LUA_CFUNC(L, GetGunCoolingRate);
  PUSH_LUA_CFUNC(L, GetGunHeat);
  PUSH_LUA_CFUNC(L, GetHeading);
  PUSH_LUA_CFUNC(L, GetHeight);
  PUSH_LUA_CFUNC(L, GetLength);
  PUSH_LUA_CFUNC(L, GetName);
  PUSH_LUA_CFUNC(L, GetWidth);
  PUSH_LUA_CFUNC(L, GetVelocity);
  PUSH_LUA_CFUNC(L, GetX);
  PUSH_LUA_CFUNC(L, GetY);
  PUSH_LUA_CFUNC(L, GetZ);
  PUSH_LUA_CFUNC(L, GetDistanceRemaining);
  PUSH_LUA_CFUNC(L, GetTurnRemaining);

#if defined(HAVE_UNISTD_H) && defined(HAVE_FCNTL_H)
  PUSH_LUA_CFUNC(L, ReadStdin);
#endif

  lua_setglobal(L, "bz");

  return true;
}


//============================================================================//

static inline LuaRobot* GetRobot(lua_State* L)
{
  lua_getfield(L, LUA_REGISTRYINDEX, ThisLabel);
  if (!lua_isuserdata(L, -1)) {
    luaL_error(L, "Internal error -- missing '%s'", ThisLabel);
  }
  LuaRobot* robot = (LuaRobot*)lua_touserdata(L, -1);
  lua_pop(L, 1);
  return robot;
}


//============================================================================//

static int Execute(lua_State* L)
{
  GetRobot(L)->execute();
  return 0;
}


static int DoNothing(lua_State* L)
{
  GetRobot(L)->doNothing();
  return 0;
}


static int Fire(lua_State* L)
{
  GetRobot(L)->fire();
  return 0;
}


static int Resume(lua_State* L)
{
  GetRobot(L)->resume();
  return 0;
}


static int Scan(lua_State* L)
{
  GetRobot(L)->scan();
  return 0;
}


static int Stop(lua_State* L)
{
  GetRobot(L)->stop();
  return 0;
}


static int Ahead(lua_State* L)
{
  GetRobot(L)->ahead(luaL_checkdouble(L, 1));
  return 0;
}


static int Back(lua_State* L)
{
  GetRobot(L)->back(luaL_checkdouble(L, 1));
  return 0;
}


static int TurnLeft(lua_State* L)
{
  GetRobot(L)->turnLeft(luaL_checkdouble(L, 1));
  return 0;
}


static int TurnRight(lua_State* L)
{
  GetRobot(L)->turnRight(luaL_checkdouble(L, 1));
  return 0;
}


//============================================================================//

static int SetAhead(lua_State* L)
{
  GetRobot(L)->setAhead(luaL_checkdouble(L, 1));
  return 0;
}


static int SetFire(lua_State* L)
{
  GetRobot(L)->setFire();
  return 0;
}


static int SetTurnRate(lua_State* L)
{
  GetRobot(L)->setTurnRate(luaL_checkdouble(L, 1));
  return 0;
}


static int SetMaxVelocity(lua_State* L)
{
  GetRobot(L)->setMaxVelocity(luaL_checkdouble(L, 1));
  return 0;
}


static int SetResume(lua_State* L)
{
  GetRobot(L)->setResume();
  return 0;
}


static int SetStop(lua_State* L)
{
  if (lua_type(L, 1) != LUA_TBOOLEAN) {
    GetRobot(L)->setStop();
  } else {
    GetRobot(L)->setStop(lua_toboolean(L, 1));
  }
  return 0;
}


static int SetTurnLeft(lua_State* L)
{
  GetRobot(L)->setTurnLeft(luaL_checkdouble(L, 1));
  return 0;
}


//============================================================================//

static int GetTime(lua_State* L)
{
  lua_pushdouble(L, GetRobot(L)->getTime());
  return 1;
}


static int GetBattleFieldSize(lua_State* L)
{
  lua_pushdouble(L, GetRobot(L)->getBattleFieldSize());
  return 1;
}


static int GetGunCoolingRate(lua_State* L)
{
  lua_pushdouble(L, GetRobot(L)->getGunCoolingRate());
  return 1;
}


static int GetGunHeat(lua_State* L)
{
  lua_pushdouble(L, GetRobot(L)->getGunHeat());
  return 1;
}


static int GetHeading(lua_State* L)
{
  lua_pushdouble(L, GetRobot(L)->getHeading());
  return 1;
}


static int GetHeight(lua_State* L)
{
  lua_pushdouble(L, GetRobot(L)->getHeight());
  return 1;
}


static int GetLength(lua_State* L)
{
  lua_pushdouble(L, GetRobot(L)->getLength());
  return 1;
}


static int GetName(lua_State* L)
{
  lua_pushstring(L, GetRobot(L)->getName());
  return 1;
}


static int GetWidth(lua_State* L)
{
  lua_pushdouble(L, GetRobot(L)->getWidth());
  return 1;
}


static int GetVelocity(lua_State* L)
{
  lua_pushdouble(L, GetRobot(L)->getVelocity());
  return 1;
}


static int GetX(lua_State* L)
{
  lua_pushdouble(L, GetRobot(L)->getX());
  return 1;
}


static int GetY(lua_State* L)
{
  lua_pushdouble(L, GetRobot(L)->getY());
  return 1;
}


static int GetZ(lua_State* L)
{
  lua_pushdouble(L, GetRobot(L)->getZ());
  return 1;
}


static int GetDistanceRemaining(lua_State* L)
{
  lua_pushdouble(L, GetRobot(L)->getDistanceRemaining());
  return 1;
}


static int GetTurnRemaining(lua_State* L)
{
  lua_pushdouble(L, GetRobot(L)->getTurnRemaining());
  return 1;
}


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
