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

#include "KeyMap.h"
#include <ctype.h>

const BzfKeyEvent	BzfKeyMap::defaults1[] = {
				{ 0, BzfKeyEvent::LeftMouse, 0 },
				{ 0, BzfKeyEvent::MiddleMouse, 0 },
				{ 0, BzfKeyEvent::RightMouse, 0 },
				{ '1', 0, 0 },
				{ '2', 0, 0 },
				{ '3', 0, 0 },
				{ 'n', 0, 0 },
				{ 'm', 0, 0 },
				{ ',', 0, 0 },
				{ '.', 0, 0 },
				{ '\t', 0, 0 },
				{ 'b', 0, 0 },
				{ 's', 0, 0 },
				{ 'l', 0, 0 },
				{ 'f', 0, 0 },
				{ '=', 0, 0 },
				{ '-', 0, 0 },
				{ 0, BzfKeyEvent::Pause, 0 },
				{ 0, BzfKeyEvent::Delete, 0 },
				{ 0, BzfKeyEvent::F12, 0 },
				{ 0, BzfKeyEvent::PageUp, 0 },
				{ 0, BzfKeyEvent::PageDown, 0 },
				{ 'a', 0, 0 },
				{ 'h',0,0},
				{ 'j',0,0},
				{ 'k',0,0},
				{ 'o',0,0}
			};
const BzfKeyEvent	BzfKeyMap::defaults2[] = {
				{ '\r', 0, 0 },
				{ ' ', 0, 0 },
				{ 'i', 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ '+', 0, 0 },
				{ '_', 0, 0 },
				{ 'p', 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 }
			};
const char*		BzfKeyMap::keyName[] = {
				"fireKey",
				"dropFlagKey",
				"identifyKey",
				"shortRangeKey",
				"mediumRangeKey",
				"longRangeKey",
				"sendAllKey",
				"sendTeamKey",
				"sendNemesisKey",
				"sendRecipientKey",
				"jumpKey",
				"binocularsKey",
                                "scoreKey",
				"labelsKey",
				"flagHelpKey",
				"timeForwardKey",
				"timeBackwardKey",
				"pauseKey",
				"destructKey",
				"quitKey",
				"scrollBackwardKey",
				"scrollForwardKey",
				"slowKeyboardMotion",
				"hideRadarFlagsKey",
				"hideMainFlagsKey",
				"chooseSilenceKey",
				"serverCommandKey"
			};

const char*		BzfKeyMap::eventNames[] = {
				"???",
				"Pause",
				"Home",
				"End",
				"Left Arrow",
				"Right Arrow",
				"Up Arrow",
				"Down Arrow",
				"Page Up",
				"Page Down",
				"Insert",
				"Delete",
				"F1",
				"F2",
				"F3",
				"F4",
				"F5",
				"F6",
				"F7",
				"F8",
				"F9",
				"F10",
				"F11",
				"F12",
				"Left Mouse",
				"Middle Mouse",
				"Right Mouse",
				"Button 4",
				"Button 5",
				"Button 6",
				"Button 7",
				"Button 8",
				"Button 9"
			};

BzfKeyMap::BzfKeyMap()
{
  resetAll();
}

BzfKeyMap::~BzfKeyMap()
{
  // do nothing
}

void			BzfKeyMap::resetAll()
{
  reset(FireShot);
  reset(DropFlag);
  reset(Identify);
  reset(ShortRange);
  reset(MediumRange);
  reset(LongRange);
  reset(SendAll);
  reset(SendTeam);
  reset(SendNemesis);
  reset(SendRecipient);
  reset(Jump);
  reset(Binoculars);
  reset(Score);
  reset(Labels);
  reset(FlagHelp);
  reset(TimeForward);
  reset(TimeBackward);
  reset(Pause);
  reset(Destruct);
  reset(Quit);
  reset(ScrollBackward);
  reset(ScrollForward);
  reset(SlowKeyboardMotion);
  reset(ToggleRadarFlags);
  reset(ToggleMainFlags);
  reset(ChooseSilence);
  reset(ServerCommand);
}

void			BzfKeyMap::reset(Key key)
{
  clear(key);
  set(key, defaults1[key]);
  set(key, defaults2[key]);
}

void			BzfKeyMap::clear(Key key)
{
  map1[key].ascii = 0;
  map1[key].button = 0;
  map1[key].shift = 0;
  map2[key].ascii = 0;
  map2[key].button = 0;
  map2[key].shift = 0;
}

void			BzfKeyMap::set(Key key, const BzfKeyEvent& event)
{
  if ((map1[key].ascii != 0 || map1[key].button != 0) &&
      (map2[key].ascii != 0 || map2[key].button != 0))
    clear(key);

  if (map1[key].ascii == 0 && map1[key].button == 0) {
    map1[key].ascii = toupper(event.ascii);
    map1[key].button = event.button;
  }
  else {
    map2[key].ascii = toupper(event.ascii);
    map2[key].button = event.button;
  }
}

void			BzfKeyMap::unset(Key key, const BzfKeyEvent& event)
{
  if (event.ascii == 0 && event.button == 0) return;
  if (map1[key].ascii == toupper(event.ascii) &&
      map1[key].button == event.button) {
    map1[key] = map2[key];
    map2[key].ascii = 0;
    map2[key].button = 0;
  }
  else if (map2[key].ascii == toupper(event.ascii) &&
	   map2[key].button == event.button) {
    map2[key].ascii = 0;
    map2[key].button = 0;
  }
}

const BzfKeyEvent&	BzfKeyMap::get(Key key) const
{
  return map1[key];
}

const BzfKeyEvent&	BzfKeyMap::getAlternate(Key key) const
{
  return map2[key];
}

BzfKeyMap::Key		BzfKeyMap::isMapped(char ascii) const
{
  BzfKeyEvent event;
  event.ascii = ascii;
  event.button = 0;
  event.shift = 0;
  return isMapped(event);
}

BzfKeyMap::Key		BzfKeyMap::isMapped(BzfKeyEvent::Button button) const
{
  BzfKeyEvent event;
  event.ascii = 0;
  event.button = button;
  event.shift = 0;
  return isMapped(event);
}

BzfKeyMap::Key		BzfKeyMap::isMapped(const BzfKeyEvent& event) const
{
  if (event.ascii == 0 && event.button == 0)
    return LastKey;

  for (int i = 0; i < LastKey; i++) {
    if (map1[i].ascii == toupper(event.ascii) &&
	map1[i].button == event.button)
      return (Key)i;
    if (map2[i].ascii == toupper(event.ascii) &&
	map2[i].button == event.button)
      return (Key)i;
  }
  return LastKey;
}

bool			BzfKeyMap::isMappedTo(Key key,
				const BzfKeyEvent& event) const
{
  if (event.ascii == 0 && event.button == 0)
    return false;

  if (map1[key].ascii == toupper(event.ascii) &&
      map1[key].button == event.button)
    return true;
  if (map2[key].ascii == toupper(event.ascii) &&
      map2[key].button == event.button)
    return true;

  return false;
}

std::string		BzfKeyMap::getKeyName(Key key)
{
  return std::string(keyName[key]);
}

BzfKeyMap::Key		BzfKeyMap::lookupKeyName(const std::string& name)
{
  for (int i = 0; i < (int)(sizeof(keyName) / sizeof(keyName[0])); i++)
    if (name == keyName[i])
      return (Key)i;
  return LastKey;
}

std::string		BzfKeyMap::getKeyEventString(const BzfKeyEvent& event)
{
  if (event.ascii != 0) {
    if (event.ascii == '\b') return "Backspace";
    if (!isspace(event.ascii)) return std::string(&event.ascii, 1);
    if (event.ascii == ' ') return "Space";
    if (event.ascii == '\t') return "Tab";
    if (event.ascii == '\r') return "Enter";
    return "???";
  }

  return eventNames[event.button];
}

bool			BzfKeyMap::translateStringToEvent(
				const std::string& value, BzfKeyEvent& event)
{
  event.shift = 0;

  // ignore bogus value
  if (value == "???") return false;

  // check for plain ascii (one character) values
  if (value.size() == 1) {
    char c = value[0];
    if (isalnum(c) || ispunct(c)) {
      event.ascii = c;
      event.button = 0;
      return true;
    }
    return false;
  }

  // check whitespace values
  if (value == "Space") {
    event.ascii = ' ';
    event.button = 0;
    return true;
  }
  if (value == "Tab") {
    event.ascii = '\t';
    event.button = 0;
    return true;
  }
  if (value == "Backspace") {
    event.ascii = '\b';
    event.button = 0;
    return true;
  }
  if (value == "Enter") {
    event.ascii = '\r';
    event.button = 0;
    return true;
  }

  // check non-ascii button strings
  for (int i = 0; i < (int)(sizeof(eventNames) / sizeof(eventNames[0])); i++)
    if (value == eventNames[i]) {
      event.ascii = 0;
      event.button = i;
      return true;
    }

  return false;
}
// ex: shiftwidth=2 tabstop=8
