/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* BSPSceneDatabase:
 *	BSP tree database of geometry to render
 */

#ifndef	BZF_BSP_SCENE_DATABASE_H
#define	BZF_BSP_SCENE_DATABASE_H

// common goes first
#include "common.h"

// system headers
#include <vector>

// common implementation headers
#include "bzfgl.h"
#include "SceneDatabase.h"


class BSPSceneDatabase : public SceneDatabase {
  public:
			BSPSceneDatabase();
			~BSPSceneDatabase();

    void		addStaticNode(SceneNode*);
    void		addDynamicNode(SceneNode*);
    void		addDynamicSphere(SphereSceneNode*);
    void		removeDynamicNodes();
    void		removeAllNodes();
    bool		isOrdered();

    void		updateNodeStyles();
    void		addLights(SceneRenderer& renderer);
    void		addShadowNodes(SceneRenderer &renderer);
    void		addRenderNodes(SceneRenderer& renderer);
    
  private:
    class Node {
      public:
			Node(bool dynamic, SceneNode* node);
      public:
	bool		dynamic;
	int		count;
	SceneNode*	node;
	Node*		front;
	Node*		back;
    };

    void		setNodeStyle(Node*);
    void		nodeAddLights(Node*);
    void		nodeAddShadowNodes(Node*);
    void		nodeAddRenderNodes(Node*);
    
    void		insertStatic(int, Node*, SceneNode*);
    void		insertDynamic(int, Node*, SceneNode*);
    void		removeDynamic(Node*);
    void		free(Node*);
    void		release(Node*);
    void		setDepth(int newDepth);

  private:
    Node*		root;
    int			depth;
    // the following members avoid passing parameters around
    GLfloat		eye[3];
    SceneRenderer*	renderer;
    const ViewFrustum*	frustum;
};


#endif // BZF_BSP_SCENE_DATABASE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

