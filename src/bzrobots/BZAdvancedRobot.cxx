#include "BZAdvancedRobot.h"
#include "MessageUtilities.h"
#include "RCRequests.h"

BZAdvancedRobot::BZAdvancedRobot() :link(NULL), compatability(true)
{
}

bool BZAdvancedRobot::getCompatability() const { return compatability; }
void BZAdvancedRobot::setCompatability(bool newState) { compatability = newState; }
void BZAdvancedRobot::setLink(RCLinkFrontend *_link)
{
  link = _link;
}

void BZAdvancedRobot::run()
{
  initialize();
  while (true)
  {
    update();
  }
}

void BZAdvancedRobot::execute() 
{
  link->sendAndProcess(ExecuteReq(), this);
}

double BZAdvancedRobot::getDistanceRemaining() const 
{
  link->sendAndProcess(GetDistanceRemainingReq(), this);
  return distanceRemaining;
}

double BZAdvancedRobot::getTurnRemaining() const 
{
  link->sendAndProcess(GetTurnRemainingReq(), this);
  return turnRemaining;
}

void BZAdvancedRobot::setAhead(double distance)
{
  link->sendAndProcess(SetAheadReq(distance), this);
}

void BZAdvancedRobot::setFire()
{
  link->sendAndProcess(SetFireReq(), this);
}

void BZAdvancedRobot::setTurnRate(double turnRate)
{
  link->sendAndProcess(SetTurnRateReq(turnRate), this);
}

void BZAdvancedRobot::setSpeed(double speed)
{
  link->sendAndProcess(SetSpeedReq(speed), this);
}

void BZAdvancedRobot::setResume()
{
  link->sendAndProcess(SetResumeReq(), this);
}

void BZAdvancedRobot::setStop()
{
  setStop(false);
}

void BZAdvancedRobot::setStop(bool overwrite)
{
  link->sendAndProcess(SetStopReq(overwrite), this);
}

void BZAdvancedRobot::setTurnLeft(double degrees)
{
  link->sendAndProcess(SetTurnLeftReq(degrees), this);
}


double BZAdvancedRobot::getBattleFieldSize() const 
{
  link->sendAndProcess(GetBattleFieldSizeReq(), this);
  return battleFieldSize;
}

// These are normally in Robot and not AdvancedRobot, but due to
// the upside-down hierarchy we have - they're here instead ;-)
double BZAdvancedRobot::getGunHeat() const 
{
  link->sendAndProcess(GetGunHeatReq(), this);
  return gunHeat;
}

double BZAdvancedRobot::getHeading() const 
{
  link->sendAndProcess(GetHeadingReq(), this);
  return heading;
}

double BZAdvancedRobot::getHeight() const 
{
  link->sendAndProcess(GetHeightReq(), this);
  return tankHeight;
}

double BZAdvancedRobot::getWidth() const 
{
  link->sendAndProcess(GetWidthReq(), this);
  return tankWidth;
}

double BZAdvancedRobot::getLength() const 
{
  link->sendAndProcess(GetLengthReq(), this);
  return tankLength;
}

void BZAdvancedRobot::getPlayers() const 
{
  link->sendAndProcess(GetPlayersReq(), this);
}

long BZAdvancedRobot::getTime() const 
{
  /* TODO: Implement this. */
  return 0;
}

double BZAdvancedRobot::getVelocity() const 
{
  /* TODO: Implement this. */
  return 0.0;
}

double BZAdvancedRobot::getX() const 
{
  link->sendAndProcess(GetXReq(), this);
  return xPosition;
}

double BZAdvancedRobot::getY() const 
{
  link->sendAndProcess(GetYReq(), this);
  return yPosition;
}

double BZAdvancedRobot::getZ() const 
{
  link->sendAndProcess(GetZReq(), this);
  return zPosition;
}

double BZAdvancedRobot::getBearing(const Tank &tank) const
{
  return getBearing(tank.position[0], tank.position[1]);
}

double BZAdvancedRobot::getBearing(double x, double y) const
{
  float vec[2] = {x - getX(), y - getY()};

  if (vec[0] == 0 && vec[1] == 0)
    return 0.0;

  // Convert to a unit vector.
  float len = sqrt(vec[0]*vec[0] + vec[1]*vec[1]);
  vec[0] /= len; vec[1] /= len;

  return MessageUtilities::overflow(atan2(vec[1], vec[0])*180.0/M_PI - getHeading(), -180.0, 180.0);

}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8