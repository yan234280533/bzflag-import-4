/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "ServerCommandKey.h"

#include "LocalPlayer.h"
#include "HUDRenderer.h"
#include "TextUtils.h"
#include "KeyManager.h"
#include "playing.h"
#include "HUDui.h"


const ServerCommandKey::Mode ServerCommandKey::nonAdminModes [8] = {LagStats, IdleStats, FlagHistory, Report, Password, Register, Identify, ClientQuery};

/* FIXME - note the important numModes and numNonAdminModes values inited here
 * when new commands are added, the indices need to be adjusted here.
 */
ServerCommandKey::ServerCommandKey()
  : mode(nonAdminModes[0])
  , startIndex(-1)
  , numModes(35)		//brittle... no good portable way to deal with this
  , numNonAdminModes( sizeof(nonAdminModes)/sizeof(nonAdminModes[0]) )
{
}

void			ServerCommandKey::nonAdminInit()
{
  // if we are in a non admin mode stay there
  bool inNonAdminCommand = false;
  for (int i = 0; i < numNonAdminModes; i++) {
    if (nonAdminModes[i] == mode) {
      inNonAdminCommand = true;
      break;
    }
  }
  if (!inNonAdminCommand)
    mode = nonAdminModes[0];

  updatePrompt();
}
void			ServerCommandKey::adminInit()
{
  updatePrompt();
}

void			ServerCommandKey::init()
{
  updatePrompt();
}

void			ServerCommandKey::updatePrompt()
{
  std::string composePrompt, banPattern;
  // decide what should be on the composing prompt

  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (!myTank) {
    // make sure we actually have a tank
    return;
  }

  bool allowEdit = true;  // default to true, only need to indicate which are false  
  const Player *recipient = myTank->getRecipient();

  switch (mode) {
    case Kick:
      if (recipient) {
	composePrompt = "Kick -> ";
	composePrompt += recipient->getCallSign();
	composePrompt += " :";
      } else {
	composePrompt = "Kick :";
      }
      break;
    case Kill:
      if (recipient) {
	composePrompt = "Kill -> ";
	composePrompt += recipient->getCallSign();
	composePrompt += " :";
      } else {
	composePrompt = "Kill :";
      }
      break;
    case BanIp:
    case Ban1:
    case Ban2:
    case Ban3:
      if (recipient) {
	// Set the prompt and enable editing/composing --> allows to enter ban time
	/* FIXME FIXME FIXME
	 * temporarily breaking bans for playerid->ubyte
	 banPattern = makePattern(recipient->id.serverHost);
	 composePrompt = "Ban " + banPattern + " -> " + recipient->getCallSign() + " :";
	*/
	return; // remove this when the above is fixed
      } else {
	composePrompt = "Ban :";
      }
      break;
    case Showgroup:
      if (recipient) {
	composePrompt = "Show player's groups -> ";
	composePrompt += recipient->getCallSign();
	allowEdit = false;
      } else {
	composePrompt = "Show player's group :";
      }
      break;
    case Setgroup:
      if (recipient) {
	composePrompt = "Set player's group -> ";
	composePrompt += recipient->getCallSign();
	composePrompt += " :";
      } else {
	composePrompt = "Set player's group :";
      }
      break;
    case Removegroup:
      if (recipient) {
	composePrompt = "Remove player from group -> ";
	composePrompt += recipient->getCallSign();
	composePrompt += " :";
      } else {
	composePrompt = "Remove player from group :";
      }
      break;
    case Ghost:
      if (recipient) {
	composePrompt = "Ghost player [enter your pass] -> ";
	composePrompt += recipient->getCallSign();
	composePrompt += " :";
      } else {
	composePrompt = "Ghost :";
      }
      break;
    case Unban:
      composePrompt = "Unban :";
      break;
    case Banlist:
      composePrompt = "Show ban list";
      allowEdit = false;
      break;
    case Playerlist:
      composePrompt = "Show player list";
      allowEdit = false;
      break;
    case FlagReset:
      composePrompt = "Reset Flags";
      allowEdit = false;
      break;
    case FlagUnusedReset:
      composePrompt = "Reset Unused Flags";
      allowEdit = false;
      break;
    case FlagUp:
      composePrompt = "Remove Flags";
      allowEdit = false;
      break;
    case FlagShow:
      composePrompt = "Show Flag Info";
      allowEdit = false;
      break;
    case FlagHistory:
      composePrompt = "Flag History";
      allowEdit = false;
      break;
    case IdleStats:
      composePrompt = "Idle Stats";
      allowEdit = false;
      break;
    case ClientQuery:
      composePrompt = "Client Version Query";
      allowEdit = false;
      break;
    case LagStats:
      composePrompt = "Lag / Ping Stats";
      allowEdit = false;
      break;
    case Report:
      composePrompt = "Send Report to Server:";
      break;
    case LagWarn:
      composePrompt = "Lag Warning threshold:";
      break;
    case LagDrop:
      composePrompt = "Maximum lag warnings:";
      break;
    case GameOver:
      composePrompt = "End Game";
      allowEdit = false;
      break;
    case CountDown:
      composePrompt = "Restart Countdown:";
      break;
    case SuperKill:
      composePrompt = "Disconnect all Players";
      allowEdit = false;
      break;
    case Shutdown:
      composePrompt = "Shut Down Server";
      allowEdit = false;
      break;
    case Register:
    case Identify:
      break; // these were missing prior to the refactor
    case Setpass:
      composePrompt = "Set your password [enter pass]:";
      break;
    case Grouplist:
      composePrompt = "List Groups";
      allowEdit = false;
      break;
    case Groupperms:
      composePrompt = "List Permissions";
      allowEdit = false;
      break;
    case Vote:
      composePrompt = "Vote:";
      break;
    case Poll:
      composePrompt = "Call a Poll:";
      break;
    case Veto:
      composePrompt = "Cancel a Poll";
      allowEdit = false;
      break;
    case Password:
      composePrompt = "Admin Password:";
      break;
    default:
      // Because of the type-safety of the enum, this should never happen
      // (unless someone adds a new mode without updating this case statement)
      return;
  }

  hud->setComposing(composePrompt, allowEdit);
}

// return the right ban pattern 123.32.12.* for example depending on the
// mode of the class. Returns an empty string on errors.
std::string		ServerCommandKey::makePattern(const InAddr& address)
{
  const char * c = inet_ntoa(address);
  if (c == NULL) return "";
  std::string dots = c;
  std::vector<std::string> dotChunks = TextUtils::tokenize(dots, ".");
  if (dotChunks.size() != 4) return "";

  switch (mode) {
    case BanIp:
      return dots;
    case Ban1:
      return dotChunks[0] + "." + dotChunks[1] + "." + dotChunks[2] + ".*";
    case Ban2:
      return dotChunks[0] + "." + dotChunks[1] + ".*.*";
    case Ban3:
      return dotChunks[0] + ".*.*.*";
    default:
      break;
  }

  return "";
}

bool			ServerCommandKey::keyPress(const BzfKeyEvent& key)
{
  bool sendIt;
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (KEYMGR.get(key, true) == "jump") {
    // jump while typing
    myTank->setJump();
  }

  if (myTank->getInputMethod() != LocalPlayer::Keyboard) {
    if ((key.button == BzfKeyEvent::Up) ||
	(key.button == BzfKeyEvent::Down) ||
	(key.button == BzfKeyEvent::Left) ||
	(key.button == BzfKeyEvent::Right))
      return true;
  }

  switch (key.ascii) {
    case 3:	// ^C
    case 27:	// escape
      // case 127:   // delete
      sendIt = false;			// finished composing -- don't send
      break;

    case 4:	// ^D
    case 13:	// return
      sendIt = true;
      break;

    default:
      return false;
  }

  if (sendIt) {
    std::string message = hud->getComposeString();
    std::string banPattern, sendMsg, displayMsg, name;

    const Player * troll = myTank->getRecipient();
    if (mode >= Kick && mode <=Ghost){ // handle more complicated modes
      if (troll) { // cases where we select recipient with keys

	name = troll->getCallSign();

	switch (mode) {

	  case Kick:
	    // escape the name
	    name = TextUtils::replace_all(name, "\\", "\\\\");
	    name = TextUtils::replace_all(name, "\"", "\\\"");
	    sendMsg="/kick \"" + name + "\"";
	    if (message != "") sendMsg = sendMsg + " " + message;
	    break;

	  case Kill:
	    // escape the name
	    name = TextUtils::replace_all(name, "\\", "\\\\");
	    name = TextUtils::replace_all(name, "\"", "\\\"");
	    sendMsg="/kill \"" + name + "\"";
	    if (message != "") sendMsg = sendMsg + " " + message;
	    break;

	  case BanIp: case Ban1: case Ban2: case Ban3:

	    /* FIXME FIXME FIXME
	     * temporarily break ban-by-name for playerid->ubyte
	     banPattern = makePattern(troll->id.serverHost);
	     sendMsg="/ban " + banPattern;

	     if (message != ""){ // add ban length if something is there
	     sendMsg = sendMsg + " " + message;
	     }
	    */
	    break;

	  case Setgroup:
	    sendMsg = "/setgroup";
	    sendMsg = sendMsg + " \"" + name + "\"" +" " + message;
	    break;
	  case Removegroup:
	    sendMsg = "/removegroup";
	    sendMsg = sendMsg + " \"" + name + "\"" +" " + message;
	    break;
	  case Ghost:
	    sendMsg = "/ghost";
	    sendMsg = sendMsg + " \"" + name + "\"" +" " + message;
	    break;
	  case Showgroup:
	    sendMsg = "/showgroup";
	    sendMsg = sendMsg + " \"" + name + "\"";
	    break;

	  default: /* shouldn't happen */ break;

	}

      } else { // no recipient -- editing mode

	switch (mode) {
	  case Kick: sendMsg = "/kick"; break;
	  case Kill: sendMsg = "/kill"; break;
	  case BanIp: sendMsg = "/ban"; break;
	  case Setgroup: sendMsg = "/setgroup"; break;
	  case Removegroup: sendMsg = "/removegroup"; break;
	  case Ghost: sendMsg = "/ghost"; break;
	  case Showgroup: sendMsg = "/showgroup"; break;
	  default: /* shouldn't happen */ break;
	}

	if (message != "") sendMsg = sendMsg + " " + message;

      }
    } else { // handle less complicated messages
      switch (mode) {
	case Unban: sendMsg = "/unban " + message; break;
	case Banlist: sendMsg = "/banlist";  break;
	case Playerlist: sendMsg = "/playerlist";  break;
	case FlagReset: sendMsg = "/flag reset"; break;
	case FlagUnusedReset: sendMsg = "/flag reset unused"; break;
	case FlagUp: sendMsg = "/flag up"; break;
	case GameOver: sendMsg = "/gameover"; break;
	case CountDown: sendMsg = "/countdown " + message; break;
	case FlagShow: sendMsg = "/flag show"; break;
	case Shutdown: sendMsg = "/shutdownserver"; break;
	case SuperKill: sendMsg = "/superkill"; break;
	case LagWarn: sendMsg = "/lagwarn " + message; break;
	case LagDrop: sendMsg = "/lagdrop " + message; break;
	case IdleStats: sendMsg = "/idlestats"; break;
	case LagStats: sendMsg = "/lagstats"; break;
	case ClientQuery: sendMsg = "/clientquery"; break;
	case FlagHistory: sendMsg = "/flaghistory"; break;
	case Password: sendMsg = "/password "+ message; break;
	case Report: sendMsg = "/report "+ message; break;
	case Identify: sendMsg = "/identify "+ message; break;
	case Setpass: sendMsg = "/setpass "+ message; break;
	case Grouplist: sendMsg = "/grouplist"; break;
	case Groupperms: sendMsg = "/groupperms"; break;
	case Vote: sendMsg = "/vote " + message; break;
	case Poll: sendMsg = "/poll " + message; break;
	case Veto: sendMsg = "/veto " + message; break;
	default: /* shouldn't happen */ break;
      }

    }

    // send the message on its way if it isn't empty
    if (sendMsg != "") {
      displayMsg = "-> \"" + sendMsg + "\"";
      if (sendMsg.find("/password", 0) == std::string::npos)
	addMessage(NULL, displayMsg, 2);

      char messageBuffer[MessageLen];
      memset(messageBuffer, 0, MessageLen);
      strncpy(messageBuffer, sendMsg.c_str(), MessageLen);
      serverLink->sendMessage(ServerPlayer, messageBuffer);
    }
  }

  hud->setComposing(std::string());
  HUDui::setDefaultKey(NULL);
  return true;
}

bool			ServerCommandKey::keyRelease(const BzfKeyEvent& key)
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank->getInputMethod() != LocalPlayer::Keyboard) {

    if (key.button == BzfKeyEvent::Up || key.button == BzfKeyEvent::Down
	|| key.button == BzfKeyEvent::Left || key.button == BzfKeyEvent::Right) {
      if (key.button == BzfKeyEvent::Left || key.button == BzfKeyEvent::Right)
	// robot stay on the recipient list - to ban eventually ??
	selectNextRecipient(key.button == BzfKeyEvent::Right, true);
      const Player *recipient = myTank->getRecipient();

      // choose which mode we are in
      int maxModes;
      if (myTank->isAdmin()) {
	maxModes = numModes;
      } else {
	maxModes = numNonAdminModes;
      }

      if (key.button == BzfKeyEvent::Down) {
	int newMode = mode;
	if (!myTank->isAdmin()) {
	  bool foundIt = false;
	  for (int i = 0; i < numNonAdminModes; i++) {
	    if (mode == nonAdminModes[i]) {
	      newMode = i;
	      foundIt = true;
	    }
	  }
	  if (!foundIt) newMode = 0;
	}

	newMode++;
	if (newMode >= maxModes) newMode = 0;
	mode = (myTank->isAdmin() ? ((Mode)newMode) : nonAdminModes[newMode]);
	// if no recipient skip Ban1,2,3 -- applies to admin mode
	if (!recipient && (mode >= Ban1 && mode <= Ban3))
	  mode = Unban;

      } else if (key.button == BzfKeyEvent::Up) {
	int newMode = (int) mode;

	bool foundIt = false;
	if (!myTank->isAdmin()) {
	  for (int i = 0; i < numNonAdminModes; i++) {
	    if (mode == nonAdminModes[i]) {
	      newMode = i;
	      foundIt = true;
	    }
	  }
	  if (!foundIt) newMode = 0;
	}

	newMode--;
	if (newMode < 0) newMode = maxModes -1;
	mode = (myTank->isAdmin() ? ((Mode) newMode) : nonAdminModes[newMode]);
	// if no recipient skip Ban1,2,3 -- applies to admin mode
	if (!recipient && (mode >= Ban1 && mode <= Ban3))
	  mode = BanIp;
      }

      //update composing prompt
      updatePrompt();
      return false;
    }
  }
  return keyPress(key);
}
