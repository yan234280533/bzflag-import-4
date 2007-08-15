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

#ifndef BZROBOTS_PYTHONLOADER_H
#define BZROBOTS_PYTHONLOADER_H

#include <Python.h>

#include "ScriptLoader.h"

class PythonLoader : public ScriptLoader {
  PyObject *module, *ctor;
  PyObject *robot;

  bool initialized;
  bool initialize();

  public:
    PythonLoader();
    ~PythonLoader();
    bool load(std::string filename);
    BZAdvancedRobot *create(void);
    void destroy(BZAdvancedRobot *instance);
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
