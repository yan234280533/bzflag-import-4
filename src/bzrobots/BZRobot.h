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

#ifndef __BZROBOT_H__
#define __BZROBOT_H__

#include "common.h"

/* local interface headers */
#include "BZRobotCallbacks.h"
#include "BZRobotEvents.h"

class BZRobot
{
protected:
  mutable BZRobotCallbacks *bzrobotcb;
  std::string robotType;
public:
  BZRobot();
  virtual ~BZRobot();
  void setCallbacks(BZRobotCallbacks *_bzrobotcb);
  std::string getRobotType() { return robotType; }

  void ahead(double distance);
  void doNothing();
  void fire();
  double getBattleFieldSize() const;
  double getGunHeat() const;
  double getHeading() const;
  double getHeight() const;
  double getWidth() const;
  double getLength() const;
  long getTime() const;
  double getVelocity() const;
  double getX() const;
  double getY() const;
  double getZ() const;
  void resume();
  void stop();
  void stop(bool overwrite);
  void turnLeft(double degrees);
  
  virtual void run() { printf("BZRobot: run\n"); }

  /* Event Handlers. */
  virtual void onHitWall(const HitWallEvent &/*event*/) {}
  virtual void onDeath(const DeathEvent &/*event*/) {}

  //virtual void onSkippedTurn(const SkippedTurnEvent &/*event*/) {}
  //virtual void onBulletHit(const BulletHitEvent &/*event*/) {}
  //This method is called when one of your bullets hits another robot.
  //virtual void onBulletHitBullet(const BulletHitBulletEvent &/*event*/) {}
  //This method is called when one of your bullets hits another bullet.
  //virtual void onBulletMissed(const BulletMissedEvent &/*event*/) {}
  //This method is called when one of your bullets misses, i.e. hits a wall.
  //This method is called if your robot dies.
  //virtual void onHitByBullet(const HitByBulletEvent &/*event*/) {}
  //This method is called when your robot is hit by a bullet.
  //virtual void onHitRobot(const HitRobotEvent &/*event*/) {}
  //This method is called when your robot collides with another robot.
  //virtual void onRobotDeath(const RobotDeathEvent &/*event*/) {}
  //This method is called when another robot dies.
  //virtual void onScannedRobot(const ScannedRobotEvent &/*event*/) {}
  //This method is called when your robot sees another robot, i.e. when the robot's radar scan "hits" another robot.
  //virtual void onWin(const WinEvent &/*event*/) {}
  //This method is called if your robot wins a battle.
};

#else
class BZRobot;
#endif /* __BZROBOT_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
