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

#ifndef CURSESUI_H
#define CURSESUI_H

#include <iostream>
#include <map>
#include <string>

#include "config.h"

// which curses?
#ifdef HAVE_CURSES_H
#include <curses.h>
#else
#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#else
#include "pdcurses_adapter.h"
#endif
#endif

#include "Address.h"
#include "AutoCompleter.h"
#include "BZAdminUI.h"
#include "global.h"
#include "UIMap.h"

#define CMDLENGTH (MessageLen - 3)


/** This class is an interface for bzadmin that uses ncurses. */
class CursesUI : public BZAdminUI {
protected:

  /** The parameters to this constructor are a map of all players and the
      local player's PlayerId. */
  CursesUI(const std::map<PlayerId, std::string>& p, PlayerId m);

public:

  ~CursesUI();

  /** This function prints a message in the main window. */
  virtual void outputMessage(const std::string& msg);

  /** See if the user has entered a command, if it has, store it in str and
      return true. */
  virtual bool checkCommand(std::string& str);

  /** Tell the UI that a player has been added. */
  virtual void addedPlayer(PlayerId p);

  /** Warn the UI that a player will be removed. */
  virtual void removingPlayer(PlayerId p);

  /** Get the current target (the player that messages should be sent to,
      or 0 for public messages). */
  virtual PlayerId getTarget() const;

  /** This function returns a pointer to a dynamically allocated
      CursesUI object. */
  static BZAdminUI* creator(const std::map<PlayerId, std::string>& players, PlayerId me);

protected:

  void updateTargetWin();
  void updateCmdWin();

  WINDOW* mainWin;
  WINDOW* targetWin;
  WINDOW* cmdWin;
  std::string cmd;
  const std::map<PlayerId, std::string>& players;
  std::map<PlayerId, std::string>::const_iterator targetIter;
  PlayerId me;
  AutoCompleter comp;
  std::vector<std::string> history;
  unsigned int maxHistory;
  unsigned int currentHistory;

  static UIAdder uiAdder;
};

#endif

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
