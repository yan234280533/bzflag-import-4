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

#ifdef _WIN32
#pragma warning( 4: 4786)
#endif
#include <iostream>
#ifdef _WIN32
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wincon.h>
#else
#include <sys/select.h>
#endif

#include "StdBothUI.h"
#include "global.h"

unsigned long __stdcall winInput(void *that)
{
  StdBothUI *input = (StdBothUI*)that;
  unsigned long numRead;

  while (WaitForSingleObject(input->processedEvent, INFINITE) == WAIT_OBJECT_0) {
    numRead = 0;
    ReadFile(input->console, &input->buffer[input->pos], MessageLen-input->pos, &numRead, NULL);
    if (numRead > 0) {
      input->pos += numRead;
      SetEvent(input->readEvent);
    }
  }
  return 0;
}

// add this UI to the map
UIAdder StdBothUI::uiAdder("stdboth", &StdBothUI::creator);

StdBothUI::StdBothUI()
{
#ifdef _WIN32
  unsigned long tid;
  console = GetStdHandle(STD_INPUT_HANDLE);
  readEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
  processedEvent = CreateEvent(NULL,FALSE,TRUE,NULL);
  thread = CreateThread(NULL,0,winInput,this,0,&tid);
  pos = 0;
#endif
}

void StdBothUI::outputMessage(const string& msg) {
  std::cout<<msg<<endl;
}


#ifdef _WIN32

bool StdBothUI::checkCommand(string& str) {
  if (WaitForSingleObject(readEvent, 100) == WAIT_OBJECT_0) {
    if (pos > 2) {
      if ((buffer[pos-1] == '\n') || (buffer[pos-1] == '\r') || (pos == MessageLen)) {
        buffer[pos-2] = '\0';
        str = buffer;
	pos = 0;
	SetEvent(processedEvent);
	return true;
      }
    }
    SetEvent(processedEvent);
  }
  return false;
}

#else

bool StdBothUI::checkCommand(string& str) {
  static char buffer[MessageLen + 1];
  static int pos = 0;

  fd_set rfds;
  timeval tv;
  FD_ZERO(&rfds);
  FD_SET(0, &rfds);
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  if (select(1, &rfds, NULL, NULL, &tv) > 0) {
    read(0, &buffer[pos], 1);
    if (buffer[pos] == '\n' || pos == MessageLen - 1) {
      buffer[pos] = '\0';
      str = buffer;
      if (pos != 0) {
	pos = 0;
	return true;
      }
      pos = 0;
    }
    pos++;
  }
  return false;
}
#endif


BZAdminUI* StdBothUI::creator(const map<PlayerId, string>&, PlayerId) {
  return new StdBothUI();
}

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
