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

#ifndef LUA_DYNCOL_H
#define LUA_DYNCOL_H

struct lua_State;


class LuaDynCol {
  public:
    static bool PushEntries(lua_State* L);

  private: // call-outs
    static int GetDynColName(lua_State* L);
    static int GetDynCol(lua_State* L);
    static int SetDynCol(lua_State* L);
    static int GetDynColCanHaveAlpha(lua_State* L);
};


#endif // LUA_DYNCOL_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
