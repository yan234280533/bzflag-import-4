/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "CustomWeapon.h"


/* system headers */
#include <sstream>
#include <string>
#include <math.h>

/* local implementation headers */
#include "WorldWeapons.h"
#include "TextUtils.h"

TimeKeeper CustomWeapon::sync = TimeKeeper::getCurrent();

const float CustomWeapon::minWeaponDelay = 0.1f;

CustomWeapon::CustomWeapon()
{
  pos[0] = pos[1] = pos[2] = 0.0f;
  rotation = 0.0f;
  size[0] = size[1] = size[2] = 1.0f;
  tilt = 0.0f;
  initdelay = 10.0f;
  delay.push_back(10.0f);
  type = Flags::Null;
  
  triggerType = eNullEvent;
  eventTeam = -1;
}

bool CustomWeapon::read(const char *cmd, std::istream& input) {
  if (strcmp(cmd, "initdelay") == 0) {
    input >> initdelay;
  }
  else if (strcmp(cmd, "delay") == 0) {
    std::string args;
    float d;

    delay.clear();
    std::getline(input, args);
    std::istringstream  parms(args);

    while (parms >> d) {
      if (d < minWeaponDelay) {
	std::cout << "skipping weapon delay of " << d << " seconds" << std::endl;
	continue;
      }
      else {
	delay.push_back(d);
      }
    }
    input.putback('\n');
    if (delay.size() == 0)
      return false;
  }
  else if (strcmp(cmd, "type") == 0) {
    std::string abbv;
    input >> abbv;
    type = Flag::getDescFromAbbreviation(abbv.c_str());
    if (type == NULL)
      return false;
  }
  else if (strcmp(cmd, "tilt") == 0) {
    if (!(input >> tilt)) {
      std::cout << "weapon tilt requires a value" << std::endl;
    }
    // convert to radians
    tilt = (float)(tilt * (M_PI / 180.0));
  }
  else if (strcmp(cmd, "trigger") == 0) 
  {
	  std::string triggerName;
	  input >> triggerName;

	  triggerType = eNullEvent;

	  TextUtils::tolower(triggerName);
	  if ( triggerName == "oncap")
		  triggerType = eCaptureEvent;
	  else if ( triggerName == "onspawn")
		  triggerType = ePlayerSpawnEvent;
	  else if ( triggerName == "ondie")
		  triggerType = ePlayerDieEvent;
	  else
	  {
		  std::cout << "weapon trigger type:" << triggerName << " unknown" << std::endl;
	  }
  }
  else if (strcmp(cmd, "eventteam") == 0) 
  {
	  input >> eventTeam;
  }
  else if (!WorldFileLocation::read(cmd, input)) {
    return false;
  }

  return true;
}

void CustomWeapon::writeToWorld(WorldInfo* world) const
{
	if (triggerType == eNullEvent)
		world->addWeapon(type, pos, rotation, tilt, initdelay, delay, sync);
	else
		worldEventManager.addEvent(triggerType,eventTeam,new WorldWeaponGlobalEventHandler(type, pos, rotation, tilt));
}


// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
