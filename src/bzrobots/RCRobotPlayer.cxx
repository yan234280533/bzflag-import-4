/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Remote Control Robot Player
 */

// interface header
#include "RCRobotPlayer.h"

// common implementation headers
#include "BZDBCache.h"

// local implementation headers
#include "World.h"
#include "Intersect.h"
#include "TargetingUtils.h"

RCRobotPlayer::RCRobotPlayer(const PlayerId& _id, const char* _name,
				ServerLink* _server, RCLink* _agent,
				const char* _email = "anonymous") :
				RobotPlayer(_id, _name, _server, _email),
				agent(_agent),
                                lastTickAt(0.0), tickDuration(2.0),
				speed(1.0), nextSpeed(1.0),
				turnRate(1.0), nextTurnRate(1.0),
				shoot(false),
                                distanceRemaining(0.0), nextDistance(0.0),
                                turnRemaining(0.0), nextTurn(0.0)
{
}


void			RCRobotPlayer::doUpdate(float dt)
{
  LocalPlayer::doUpdate(dt);
}

void			RCRobotPlayer::doUpdateMotion(float dt)
{
  if (isAlive()) {
    double timeNow = TimeKeeper::getCurrent().getSeconds();
    /* Is the tick still running? */
    if (lastTickAt + tickDuration >= timeNow)
    {
      if (distanceRemaining > 0.0f)
      {
        if (distanceForward)
        {
          setDesiredSpeed(speed);
          distanceRemaining -= *getVelocity() * dt;
        }
        else
        {
          setDesiredSpeed(-speed);
          distanceRemaining += *getVelocity() * dt;
        }
      }
      else
      {
          setDesiredSpeed(0);
      }

      if (turnRemaining > 0.0f)
      {
        if (turnLeft)
        {
          setDesiredAngVel(turnRate);
          turnRemaining -= getAngularVelocity() * dt;
        }
        else
        {
          setDesiredAngVel(-turnRate);
          turnRemaining += getAngularVelocity() * dt;
        }
      }
      else
      {
          setDesiredAngVel(0);
      }
    }
    else
    {
      setDesiredAngVel(0);
      setDesiredSpeed(0);
    }

  }
  LocalPlayer::doUpdateMotion(dt);
}

void			RCRobotPlayer::explodeTank()
{
  LocalPlayer::explodeTank();
}

void			RCRobotPlayer::restart(const float* pos, float _azimuth)
{
  LocalPlayer::restart(pos, _azimuth);
}

bool                    RCRobotPlayer::isInTick()
{
    double timeNow = TimeKeeper::getCurrent().getSeconds();
    /* last tick done? */
    if (lastTickAt + tickDuration >= timeNow)
      return true;
    return false;
}

bool			RCRobotPlayer::processrequest(RCRequest* req,
							    RCLink* link)
{
  receivedUpdates[req->get_request_type()] = true;
  switch (req->get_request_type()) {
    case setSpeed:
      nextSpeed = req->speed;
      link->respond("ok\n");
      break;

    case setTurnRate:
      nextTurnRate = req->turnRate;
      link->respond("ok\n");
      break;

    case setFire:
      shoot = true;
      if (fireShot()) {
	link->respond("ok\n");
      } else {
	link->respond("fail\n");
      }
      break;

    case setAhead:
      nextDistance = req->distance;
      link->respond("ok\n");
      break;

    case setTurnLeft:
      nextTurn = req->turn;
      link->respond("ok\n");
      break;

    case getDistanceRemaining:
      if (isInTick())
        return false;
      link->respondf("getDistanceRemaining %f\n", distanceRemaining);
      break;

    case getTurnRemaining:
      if (isInTick())
        return false;
      link->respondf("getTurnRemaining %f\n", turnRemaining);
      break;

    case execute:
      if (isInTick())
        return false;

      lastTickAt = TimeKeeper::getCurrent().getSeconds();

      if (receivedUpdates[setTurnLeft])
      {
        turnRemaining = nextTurn;
        if (turnRemaining < 0.0f)
        {
          turnRemaining = -turnRemaining;
          turnLeft = false;
        }
        else
          turnLeft = true;
      }

      if (receivedUpdates[setAhead])
      {
        distanceRemaining = nextDistance;
        if (distanceRemaining < 0.0f)
        {
          distanceRemaining = -distanceRemaining;
          distanceForward = false;
        }
        else
          distanceForward = true;
      }

      if (receivedUpdates[setTurnRate])
        turnRate = nextTurnRate;
      if (receivedUpdates[setSpeed])
        speed = nextSpeed;

      for (int i = 0; i < RequestCount; ++i)
        receivedUpdates[i] = false;
      break;

    default:
      break;
  }
  return true;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
