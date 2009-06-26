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

/* interface header */
#include "Shot.h"

/* local implementation headers */
#include "MessageUtilities.h"
#include "RCRequest.h"
#include "RCRequests.h"

Shot::Shot() {}

Shot::Shot(uint64_t _id) : id(_id)
{
}

Shot::Shot(PlayerId _plr, uint16_t _id)
{
  id = ((uint64_t)_plr << 16) + _id;
}

Shot::~Shot() {}

PlayerId Shot::getPlayerId(void) const
{
  return (PlayerId)(id >> 16);
}

uint16_t Shot::getShotId(void) const
{
  return id & 0xffff;
}

uint64_t Shot::getId(void) const
{
  return id;
}

void Shot::setId(uint64_t _id)
{
  id = _id;
}

std::ostream& operator<<(std::ostream& os, const Shot& shot)
{
  return os << shot.getId();
}

messageParseStatus Shot::parse(char **arguments, int count)
{
  if (count != 1)
    return InvalidArgumentCount;

  if (!MessageUtilities::parse(arguments[0], id))
    return InvalidArguments;

  return ParseOk;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
