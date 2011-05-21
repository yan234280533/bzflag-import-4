/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __RCROBOTPLAYER_H__
#define __RCROBOTPLAYER_H__

#include "common.h"

/* system interface headers */
#include <vector>

/* interface header */
#include "BZRobotPlayer.h"

/* common interface headers */
#include "ServerLink.h"

/* local interface headers */
#include "Robot.h"
#include "Region.h"
#include "RegionPriorityQueue.h"
#include "RobotPlayer.h"

#if defined(HAVE_PTHREADS)
# define LOCK_PLAYER   pthread_mutex_lock(&player_lock);
# define UNLOCK_PLAYER pthread_mutex_unlock(&player_lock);
#elif defined(_WIN32)
# define LOCK_PLAYER   EnterCriticalSection(&player_lock);
# define UNLOCK_PLAYER LeaveCriticalSection(&player_lock);
#else
# define LOCK_PLAYER
# define UNLOCK_PLAYER
#endif

#ifdef _WIN32
# define SLEEP_PLAYER(ms) Sleep(ms)
#else
# define SLEEP_PLAYER(ms) (ms > 1000 ? sleep(ms/1000) : usleep(ms*1000))
#endif

/**
 * BZFlag Robot Player
 */
class BZRobotPlayer : public RobotPlayer {
  public:
    BZRobotPlayer(const PlayerId&, const char* name, ServerLink*);
    ~BZRobotPlayer();

    typedef enum {
      speedUpdate,
      turnRateUpdate,
      distanceUpdate,
      turnUpdate,
      updateCount
    } variableUpdates;

    void setRobot(BZRobots::Robot* _robot);

    void explodeTank();
    void restart(const fvec3& pos, float azimuth);
    void update(float inputDT);
    void doUpdate(float dt);
    void doUpdateMotion(float dt);

    void shotFired(const ShotPath* shot, const Player* shooter);
    void shotKilled(const ShotPath* shot, const Player* killer, const Player* victim);

    // Call-ins for BZRobot/BZAdvancedRobot
    void botAhead(double distance);
    void botBack(double distance);
    void botClearAllEvents();
    void botDoNothing();
    void botExecute();
    void botFire(double power);
    BZRobots::Bullet* botFireBullet(double power);
    std::list<BZRobots::Event> botGetAllEvents();
    double botGetBattleFieldLength();
    double botGetBattleFieldWidth();
    std::list<BZRobots::Event> botGetBulletHitBulletEvents();
    std::list<BZRobots::Event> botGetBulletHitEvents();
    std::list<BZRobots::Event> botGetBulletMissedEvents();
    double botGetDistanceRemaining();
    double botGetEnergy();
    double botGetGunCoolingRate();
    double botGetGunHeading();
    double botGetGunHeadingRadians();
    double botGetGunTurnRemaining();
    double botGetGunTurnRemainingRadians();
    double botGetGunHeat();
    double botGetHeading();
    double botGetHeadingRadians();
    double botGetHeight();
    std::list<BZRobots::Event> botGetHitByBulletEvents();
    std::list<BZRobots::Event> botGetHitRobotEvents();
    std::list<BZRobots::Event> botGetHitWallEvents();
    double botGetLength();
    std::string botGetName();
    int botGetNumRounds();
    int botGetOthers();
    double botGetRadarHeading();
    double botGetRadarHeadingRadians();
    double botGetRadarTurnRemaining();
    double botGetRadarTurnRemainingRadians();
    std::list<BZRobots::Event> botGetRobotDeathEvents();
    int botGetRoundNum();
    std::list<BZRobots::Event> botGetScannedRobotEvents();
    std::list<BZRobots::Event> botGetStatusEvents();
    double botGetTime();
    double botGetTurnRemaining();
    double botGetTurnRemainingRadians();
    double botGetVelocity();
    double botGetWidth();
    double botGetX();
    double botGetY();
    double botGetZ();
    bool botIsAdjustGunForRobotTurn();
    bool botIsAdjustRadarForGunTurn();
    bool botIsAdjustRadarForRobotTurn();
    void botResume();
    void botScan();
    void botSetAdjustGunForRobotTurn(bool independent);
    void botSetAdjustRadarForGunTurn(bool independent);
    void botSetAdjustRadarForRobotTurn(bool independent);
    void botSetAhead(double distance);
    void botSetBack(double distance);
    void botSetFire(double power);
    BZRobots::Bullet* botSetFireBullet(double power);
    void botSetMaxTurnRate(double turnRate);
    void botSetMaxVelocity(double speed);
    void botSetResume();
    void botSetStop(bool overwrite);
    void botSetTurnLeft(double turn);
    void botSetTurnLeftRadians(double turn);
    void botSetTurnRight(double turn);
    void botSetTurnRightRadians(double turn);
    void botStop(bool overwrite);
    void botTurnGunLeft(double turn);
    void botTurnGunLeftRadians(double turn);
    void botTurnGunRight(double turn);
    void botTurnGunRightRadians(double turn);
    void botTurnLeft(double turn);
    void botTurnLeftRadians(double turn);
    void botTurnRadarLeft(double turn);
    void botTurnRadarLeftRadians(double turn);
    void botTurnRadarRight(double turn);
    void botTurnRadarRightRadians(double turn);
    void botTurnRight(double turn);
    void botTurnRightRadians(double turn);

  private:
    double lastExec;
    bool inEvents;
    bool purgeQueue;
    bool didHitWall;

// Begin shared thread-safe variables
    BZRobots::Robot* robot;

    double tsBattleFieldSize;

    int tsPlayerCount;

    const std::string tsName;

    fvec3 tsTankSize;

    bool tsPendingUpdates[updateCount];
    std::list<BZRobots::Event*> tsScanQueue;  // non-prioritized
    std::list<BZRobots::Event*> tsEventQueue;  // non-prioritized

    double tsGunHeat;
    bool tsShoot;

    fvec3 tsPosition;
    double tsCurrentHeading;
    double tsCurrentSpeed, tsSpeed, tsNextSpeed;
    double tsCurrentTurnRate, tsTurnRate, tsNextTurnRate;

    double tsDistanceRemaining, tsNextDistance;
    bool tsDistanceForward, tsTurnLeft;
    double tsTurnRemaining, tsNextTurn;

    bool tsHasStopped;
    double tsStoppedDistance, tsStoppedTurn;
    bool tsStoppedForward, tsStoppedLeft;

    double tsShotReloadTime;

#if defined(HAVE_PTHREADS)
    pthread_mutex_t player_lock;
#elif defined(_WIN32)
    CRITICAL_SECTION player_lock;
#endif
// End shared thread-safe variables
};

#else
class BZRobotPlayer;
#endif // __RCROBOTPLAYER_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
