/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "SceneNodePrimitive.h"
#include "SceneVisitor.h"

//
// SceneNodePrimitive
//

static const char*		typeEnums[] = {
		"points",
		"lines",
		"lstrip",
		"lloop",
		"triangles",
		"tstrip",
		"tfan"
};

SceneNodePrimitive::SceneNodePrimitive() :
								type("type", typeEnums, countof(typeEnums)),
								index("index", 0, 0, 1)
{
	// do nothing
}

SceneNodePrimitive::~SceneNodePrimitive()
{
	// do nothing
}

bool					SceneNodePrimitive::visit(SceneVisitor* visitor)
{
	return visitor->visit(this);
}
