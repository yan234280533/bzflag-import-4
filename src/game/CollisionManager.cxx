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

/* interface header */
#include "CollisionManager.h"

/* system implementation headers */
#include <vector>
#include <math.h>
#include <stdlib.h>

/* common implementation headers */
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "Intersect.h"
#include "Ray.h"       
#include "ObstacleMgr.h"
#include "Obstacle.h"
#include "MeshObstacle.h"
#include "BoxBuilding.h" 
#include "PyramidBuilding.h"
#include "BaseBuilding.h"   
#include "Teleporter.h"     


/* static variables */

CollisionManager COLLISIONMGR; // the big dog

static const float testFudge = 0.1f;

static int maxDepth = 0;
static int minElements = 0;

static int leafNodes = 0;
static int totalNodes = 0;
static int totalElements = 0;

static ObsList	FullList;  // the complete list of obstacles
static SplitObsList	SplitList; // the complete split list of obstacles
static ObsList	FullPad;   // for returning a full list of obstacles
static SplitObsList	SplitPad;  // for returning a split list of obstacles

static ColDetNodeList	RayList;   // for returning a list a ray hit nodes

static ObsList		EmptyList = { 0, NULL };
static ColDetNodeList	EmptyNodeList = { 0, NULL };


/* static functions */

inline static void addToFullList (Obstacle* obs)
{
  obs->collisionState = false;
  FullList.list[FullList.count] = obs;
  FullList.count++;
  return;
}

inline static void addToFullPadList (Obstacle* obs)
{
  FullPad.list[FullPad.count] = obs;
  FullPad.count++;
  return;
}

inline static void addToRayList (ColDetNode* node)
{
  RayList.list[RayList.count] = node;
  RayList.count++;
  return;
}

static void squeezeChildren (ColDetNode** children)
{
  for (int dst = 0; dst < 8; dst++) {
    if (children[dst] == NULL) {
      // replace with the next non-NULL
      for (int src = (dst + 1); src < 8; src++) {
	if (children[src] != NULL) {
	  children[dst] = children[src];
	  children[src] = NULL;
	  break;
	}
      }
    }
  }
  return;
}

static int compareZlevels (const void* a, const void* b)
{
  const Obstacle* obsA = *((const Obstacle**)a);
  const Obstacle* obsB = *((const Obstacle**)b);
  float amins[3], amaxs[3], bmins[3], bmaxs[3];
  obsA->getExtents(amins, amaxs);
  obsB->getExtents(bmins, bmaxs);
  if (fabsf(amaxs[2] - bmaxs[2]) < 1.0e-3) {
    if (amins[2] > bmins[2]) {
      return -1;
    } else {
      return +1;
    }
  }
  else if (amaxs[2] > bmaxs[2]) {
    return -1;
  }
  else {
    return +1;
  }
}


//////////////////////////////////////////////////////////////////////////////
//
// CollisionManager
//

CollisionManager::CollisionManager ()
{
  root = NULL;
  FullPad.list = NULL;
  FullList.list = NULL;
  RayList.list = NULL;
  clear();
}


CollisionManager::~CollisionManager ()
{
  clear();
}


void CollisionManager::clear ()
{
  delete root;
  root = NULL;

  WorldSize = 0.0f;
  MaxHeight = 0.0f;

  leafNodes = 0;
  totalNodes = 0;
  totalElements = 0;
  mins[0] = mins[1] = mins[2] = +MAXFLOAT;
  maxs[0] = maxs[1] = maxs[2] = -MAXFLOAT;

  delete[] FullPad.list;
  delete[] FullList.list;
  delete[] RayList.list;
  FullPad.list = NULL;
  FullPad.count = 0;
  FullList.list = NULL;
  FullList.count = 0;
  RayList.list = NULL;
  RayList.count = 0;

  for (int i = 0; i < 5; i++) {
    SplitPad.array[i].list = NULL;
    SplitPad.array[i].count = 0;
    SplitList.array[i].list = NULL;
    SplitList.array[i].count = 0;
  }

  return;
}


bool CollisionManager::needReload () const
{
  int newDepth = BZDB.evalInt (StateDatabase::BZDB_COLDETDEPTH);
  int newElements = BZDB.evalInt (StateDatabase::BZDB_COLDETELEMENTS);
  float newWorldSize = BZDB.eval (StateDatabase::BZDB_WORLDSIZE);
  if ((newDepth != maxDepth) || (newElements != minElements) ||
      (newWorldSize != WorldSize)) {
    return true;
  } else {
    return false;
  }
}


const ObsList* CollisionManager::axisBoxTest (const float* _mins,
					      const float* _maxs) const
{
  if (root == NULL) {
    return &EmptyList;
  }

  FullPad.count = 0;

  // get the list
  root->axisBoxTest (_mins, _maxs);

  // clear the collisionState on the obstacles
  for (int i = 0; i < FullPad.count; i++) {
    FullPad.list[i]->collisionState = false;
  }

  return &FullPad;
}


const ObsList* CollisionManager::cylinderTest (const float *pos,
					       float radius, float height) const
{
  if (root == NULL) {
    return &EmptyList;
  }

  float tmpMins[3], tmpMaxs[3];
  tmpMins[0] = pos[0] - radius;
  tmpMins[1] = pos[1] - radius;
  tmpMins[2] = pos[2];
  tmpMaxs[0] = pos[0] + radius;
  tmpMaxs[1] = pos[1] + radius;
  tmpMaxs[2] = pos[2] + height;

  FullPad.count = 0;

  // get the list
  root->axisBoxTest (tmpMins, tmpMaxs);

  // clear the collisionState on the obstacles
  for (int i = 0; i < FullPad.count; i++) {
    FullPad.list[i]->collisionState = false;
  }

  return &FullPad;
}


const ObsList* CollisionManager::boxTest (const float* pos, float /*angle*/,
					  float dx, float dy, float dz) const
{
  float radius = sqrtf (dx*dx + dy*dy);
  return cylinderTest (pos, radius, dz);
}


const ObsList* CollisionManager::movingBoxTest (
				  const float* oldPos, float /*oldAngle*/,
				  const float* pos, float /*angle*/,
				  float dx, float dy, float dz) const
{
  float newpos[3];

  // adjust the Z parameters for the motion
  memcpy (newpos, pos, sizeof(float[3]));
  if (oldPos[2] < pos[2]) {
    newpos[2] = oldPos[2];
    dz = dz + (pos[2] - oldPos[2]);
  } else {
    dz = dz + (oldPos[2] - pos[2]);
  }

  float radius = sqrtf (dx*dx + dy*dy);
  return cylinderTest (newpos, radius, dz);
}


const ObsList* CollisionManager::rayTest (const Ray* ray, float timeLeft) const
{
  if (root == NULL) {
    return &EmptyList;
  }

  FullPad.count = 0;
  RayList.count = 0;

  // get the list
  root->rayTest (ray, timeLeft + 0.1f);

  // clear the collisionState on the obstacles
  for (int i = 0; i < FullPad.count; i++) {
    FullPad.list[i]->collisionState = false;
  }

  return &FullPad;
}


static int compareRayNodes (const void *a, const void *b)
{
  const ColDetNode* nodeA = *((ColDetNode**)a);
  const ColDetNode* nodeB = *((ColDetNode**)b);
  return (nodeA->getInTime() > nodeB->getInTime());
}


const ColDetNodeList* CollisionManager::rayTestNodes (const Ray* ray,
						      float timeLeft) const
{
  if (root == NULL) {
    return &EmptyNodeList;
  }

  RayList.count = 0;

  // get the list
  root->rayTestNodes (ray, timeLeft + 0.1f);

  // sort the list of node
  qsort (RayList.list, RayList.count, sizeof(ColDetNode*), compareRayNodes);

  return &RayList;
}


void CollisionManager::load ()
{
  unsigned int i;

  // get the lists  
  const ObstacleList& meshes = OBSTACLEMGR.getMeshes();
  const ObstacleList& boxes = OBSTACLEMGR.getBoxes();
  const ObstacleList& pyrs = OBSTACLEMGR.getPyrs();
  const ObstacleList& bases = OBSTACLEMGR.getBases();
  const ObstacleList& teles = OBSTACLEMGR.getTeles();
  
  // clean out the cell lists
  clear();

  // setup the octree parameters
  WorldSize = BZDBCache::worldSize;
  maxDepth = BZDB.evalInt (StateDatabase::BZDB_COLDETDEPTH);
  minElements = BZDB.evalInt (StateDatabase::BZDB_COLDETELEMENTS);

  // determine the total number of obstacles
  int fullCount = 0;
  for (i = 0; i < meshes.size(); i++) {
    MeshObstacle* mesh = (MeshObstacle*) meshes[i];
    fullCount += mesh->getFaceCount() + 1; // one for the mesh itself
  }
  fullCount += (int)(boxes.size() + bases.size() +
		     pyrs.size() + (teles.size() * 3)); // 2 MeshFace links

  // get the memory for the full list and the scratch pad
  FullPad.list = new Obstacle*[fullCount];
  FullList.list = new Obstacle*[fullCount];
  FullList.count = 0;

  // add everything to the full list
  for (i = 0; i < meshes.size(); i++) {
    MeshObstacle* mesh = (MeshObstacle*) meshes[i];
    for (int f = 0; f < mesh->getFaceCount(); f++) {
      addToFullList((Obstacle*) mesh->getFace(f));
    }
  }
  for (i = 0; i < boxes.size(); i++) {
    addToFullList(boxes[i]);
  }
  for (i = 0; i < pyrs.size(); i++) {
    addToFullList(pyrs[i]);
  }
  for (i = 0; i < bases.size(); i++) {
    addToFullList(bases[i]);
  }
  for (i = 0; i < teles.size(); i++) {
    Teleporter* tele = (Teleporter*) teles[i];
    addToFullList((Obstacle*) tele);
    addToFullList((Obstacle*) tele->getBackLink());
    addToFullList((Obstacle*) tele->getFrontLink());
  }
  // FIXME - sort by Z height and size
  qsort(FullList.list, FullList.count, sizeof(Obstacle*), compareZlevels);

  // add the mesh obstacles after the sorting
  for (i = 0; i < meshes.size(); i++) {
    addToFullList(meshes[i]);
  }


  // generate the octree
  setExtents (&FullList);
  root = new ColDetNode (0, mins, maxs, &FullList);


  leafNodes = 0;
  totalNodes = 0;
  totalElements = 0;
  root->tallyStats();
  
  DEBUG2 ("ColDet Octree obstacles = %i\n", FullList.count);
  for (int i = 0; i < 3; i++) {
    DEBUG2 ("  extent[%i] = %f, %f\n", i, mins[i], maxs[i]);
  }
  DEBUG2 ("  maxHeight = %f\n", MaxHeight);
  DEBUG2 ("ColDet Octree leaf nodes  = %i\n", leafNodes);
  DEBUG2 ("ColDet Octree total nodes = %i\n", totalNodes);
  DEBUG2 ("ColDet Octree total elements = %i\n", totalElements);

  // setup the ray list
  RayList.list = new ColDetNode*[leafNodes];
  RayList.count = 0;

  // setup the split list
  Obstacle** listPtr = FullList.list;
  SplitList.named.boxes.list = listPtr;
  SplitList.named.boxes.count = (int)boxes.size();
  listPtr = listPtr + boxes.size();
  SplitList.named.bases.list = listPtr;
  SplitList.named.bases.count = (int)bases.size();
  listPtr = listPtr + bases.size();
  SplitList.named.pyrs.list = listPtr;
  SplitList.named.pyrs.count = (int)pyrs.size();
  listPtr = listPtr + pyrs.size();
  SplitList.named.teles.list = listPtr;
  SplitList.named.teles.count = (int)teles.size();

  return;
}


void CollisionManager::setExtents (ObsList *list)
{
  int i;

  mins[0] = mins[1] = mins[2] = +MAXFLOAT;
  maxs[0] = maxs[1] = maxs[2] = -MAXFLOAT;

  // find the real extents
  for (i = 0; i < list->count; i++) {
    float obsMins[3], obsMaxs[3];
    const Obstacle* obs = list->list[i];
    obs->getExtents (obsMins, obsMaxs);
    for (int a = 0; a < 3; a++) {
      if (obsMins[a] < mins[a]) {
	mins[a] = obsMins[a];
      }
      if (obsMaxs[a] > maxs[a]) {
	maxs[a] = obsMaxs[a];
      }
    }
  }
  
  // set the max height
  MaxHeight = maxs[2];

  // find the longest axis
  float width = -MAXFLOAT;
  for (i = 0; i < 3; i++) {
    float axisWidth = maxs[i] - mins[i];
    if (axisWidth > width) {
      width = axisWidth;
    }
  }

  // make it a cube, with Z on its minimum
  for (i = 0; i < 2; i++) {
    const float axisWidth = maxs[i] - mins[i];
    if (axisWidth < width) {
      const float adjust = 0.5f * (width - axisWidth);
      mins[i] = mins[i] - adjust;
      maxs[i] = maxs[i] + adjust;
    }
  }
  maxs[2] = mins[2] + width;

  return;
}


void CollisionManager::draw (DrawLinesFunc drawLinesFunc)
{
  if (root != NULL) {
    root->draw (drawLinesFunc);
  }
  return;
}


//////////////////////////////////////////////////////////////////////////////
//
// ColDetNode
//

ColDetNode::ColDetNode(unsigned char _depth,
		       const float* _mins, const float* _maxs,
		       ObsList *_list)
{
  int i;

  depth = _depth;

  for (i = 0; i < 8; i++) {
    children[i] = NULL;
  }
  childCount = 0;

  // alloacte enough room for the incoming list
  const int listBytes = _list->count * sizeof (Obstacle*);
  fullList.list = (Obstacle**) malloc (listBytes);

  // copy the extents, and make a slighty puffed up version
  float testMins[3];
  float testMaxs[3];
  for (i = 0; i < 3; i++) {
    mins[i] = _mins[i];
    maxs[i] = _maxs[i];
    testMins[i] = mins[i] - testFudge;
    testMaxs[i] = maxs[i] + testFudge;
  }

  // setup some test parameters
  float pos[3];
  pos[0] = 0.5f * (testMaxs[0] + testMins[0]);
  pos[1] = 0.5f * (testMaxs[1] + testMins[1]);
  pos[2] = testMins[2];
  float size[3];
  size[0] = 0.5f * (testMaxs[0] - testMins[0]);
  size[1] = 0.5f * (testMaxs[1] - testMins[1]);
  size[2] = (testMaxs[2] - testMins[2]);
  float point[3];
  point[0] = pos[0];
  point[1] = pos[1];
  point[2] = 0.5f * (testMaxs[2] + testMins[2]);

  // find all of the intersecting nodes
  fullList.count = 0;
  for (i = 0; i < _list->count; i++) {
    Obstacle* obs = _list->list[i];
    if (obs->getType() != MeshObstacle::getClassName()) {
      if (obs->inBox (pos, 0.0f, size[0], size[1], size[2])) {
	fullList.list[fullList.count] = obs;
	fullList.count++;
      }
    } else {
      // add a mesh if any of its faces are in the node,
      // or if it passes the 'point containment' test.
      MeshObstacle* mesh = (MeshObstacle*) obs;
      bool needCheck = true;
      for (int j = 0; j < fullList.count; j++) {
	Obstacle* tmpObs = fullList.list[j];
	if (tmpObs->getType() == MeshFace::getClassName()) {
	  MeshFace* face = (MeshFace*) tmpObs;
	  if (face->getMesh() == mesh) {
	    fullList.list[fullList.count] = (Obstacle*) mesh;
	    fullList.count++;
	    needCheck = false;
	    break;
	  }
	}
      }
      if (needCheck && mesh->containsPointNoOctree (point)) {
	fullList.list[fullList.count] = (Obstacle*) mesh;
	fullList.count++;
      }
    }
  }

  // count will remain as the total numbers of
  // scene nodes that intersect with this cell
  count = fullList.count;

  // resize the list to save space
  fullList.list = (Obstacle**) realloc (fullList.list,
					fullList.count * sizeof (Obstacle*));

  // return if this is a leaf node
  if (((int)depth >= maxDepth) || (fullList.count <= minElements)) {
    DEBUG4 ("COLDET LEAF NODE: depth = %d, items = %i\n", depth, count);
    resizeCell();
    return;
  }

  // sow the seeds
  depth++;
  makeChildren();

  // non NULLs first
  squeezeChildren (children);

  // resize this branch cell
  resizeCell();

  // clear the list
  fullList.count = 0;
  free (fullList.list);
  fullList.list = NULL;

  DEBUG4 ("COLDET BRANCH NODE: depth = %d, children = %i\n", depth, childCount);

  return;
}


ColDetNode::~ColDetNode()
{
  for (int i = 0; i < 8; i++) {
    delete children[i];
  }
  free (fullList.list);
  return;
}


void ColDetNode::makeChildren ()
{
  int side[3];    // the axis sides  (0 or 1)
  float cmins[3];
  float cmaxs[3];
  float center[3];

  // setup the center point
  for (int i = 0; i < 3; i++) {
    center[i] = 0.5f * (maxs[i] + mins[i]);
  }

  childCount = 0;
  const float* extentSet[3] = { mins, center, maxs };

  for (side[0] = 0; side[0] < 2; side[0]++) {
    for (side[1] = 0; side[1] < 2; side[1]++) {
      for (side[2] = 0; side[2] < 2; side[2]++) {

	// calculate the child's extents
	for (int a = 0; a < 3; a++) {
	  cmins[a] = extentSet[side[a]+0][a];
	  cmaxs[a] = extentSet[side[a]+1][a];
	}

	int kid = side[0] + (2 * side[1]) + (4 * side[2]);

	children[kid] = new ColDetNode (depth, cmins, cmaxs, &fullList);

	if (children[kid]->getCount() == 0) {
	  delete children[kid];
	  children[kid] = NULL;
	}
	else {
	  childCount++;
	}
      }
    }
  }

  return;
}


void ColDetNode::resizeCell ()
{
  int i;

  // resize the cell
  float absMins[3] = {+MAXFLOAT, +MAXFLOAT, +MAXFLOAT};
  float absMaxs[3] = {-MAXFLOAT, -MAXFLOAT, -MAXFLOAT};
  float tmpMins[3], tmpMaxs[3];

  for (i = 0; i < fullList.count; i++) {
    Obstacle* obs = fullList.list[i];
    obs->getExtents (tmpMins, tmpMaxs);
    for (int a = 0; a < 3; a++) {
      if (tmpMins[a] < absMins[a])
	absMins[a] = tmpMins[a];
      if (tmpMaxs[a] > absMaxs[a])
	absMaxs[a] = tmpMaxs[a];
    }
  }

  for (i = 0; i < 3; i++) {
    if (absMins[i] > mins[i])
      mins[i] = absMins[i];
    if (absMaxs[i] < maxs[i])
      maxs[i] = absMaxs[i];
  }

  return;
}


void ColDetNode::axisBoxTest (const float* _mins, const float* _maxs) const
{
  int i;

  if ((_maxs[0] < mins[0]) || (_mins[0] > maxs[0]) ||
      (_maxs[1] < mins[1]) || (_mins[1] > maxs[1]) ||
      (_maxs[2] < mins[2]) || (_mins[2] > maxs[2])) {
    return;
  }

  if (childCount == 0) {
    for (i = 0; i < fullList.count; i++) {
      Obstacle* obs = fullList.list[i];
      if (obs->collisionState == false) {
	obs->collisionState = true;
	addToFullPadList (obs);
      }
    }
  }
  else {
    for (i = 0; i < childCount; i++) {
      children[i]->axisBoxTest (_mins, _maxs);
    }
  }

  return;
}


void ColDetNode::boxTest (const float* pos, float angle,
			  float dx, float dy, float dz) const
{
  int i;

/* FIXME
  if ((_maxs[0] < mins[0]) || (_mins[0] > maxs[0]) ||
      (_maxs[1] < mins[1]) || (_mins[1] > maxs[1]) ||
      (_maxs[2] < mins[2]) || (_mins[2] > maxs[2])) {
    return;
  }
*/

  if (childCount == 0) {
    for (i = 0; i < fullList.count; i++) {
      Obstacle* obs = fullList.list[i];
      if (obs->collisionState == false) {
	obs->collisionState = true;
	addToFullPadList (obs);
      }
    }
  }
  else {
    for (i = 0; i < childCount; i++) {
      children[i]->boxTest (pos, angle, dx, dy, dz);
    }
  }

  return;
}


void ColDetNode::rayTest (const Ray* ray, float timeLeft) const
{
  if (!testRayHitsAxisBox(ray, mins, maxs, &inTime) ||
      (inTime > timeLeft)) {
    return;
  }

  if (childCount == 0) {
    for (int i = 0; i < fullList.count; i++) {
      Obstacle* obs = fullList.list[i];
      if (obs->collisionState == false) {
	obs->collisionState = true;
	addToFullPadList (obs);
      }
    }
  }
  else {
    for (int i = 0; i < childCount; i++) {
      children[i]->rayTest (ray, timeLeft);
    }
  }

  return;
}


void ColDetNode::rayTestNodes (const Ray* ray, float timeLeft) const
{
  if (!testRayHitsAxisBox(ray, mins, maxs, &inTime, &outTime) ||
      (inTime > timeLeft)) {
    return;
  }

  if (childCount == 0) {
    addToRayList((ColDetNode*)this);
  }
  else {
    for (int i = 0; i < childCount; i++) {
      children[i]->rayTestNodes (ray, timeLeft);
    }
  }

  return;
}


/*
void ColDetNode::boxTestSplit (const float* pos, float angle,
			       float dx, float dy, float dz) const
{
  pos = pos;
  angle = dx =dy =dz;
  return;
}
*/


void ColDetNode::tallyStats()
{
  totalNodes++;
  totalElements += fullList.count;

  if (childCount > 0) {
    for (int i = 0; i < childCount; i++) {
      children[i]->tallyStats();
    }
  } else {
    leafNodes++;
  }

  return;
}


void ColDetNode::draw(DrawLinesFunc drawLinesFunc)
{
  int x, y, z, c;
  float points[5][3];
  const float* extents[2] = { mins, maxs };

  // pick a color
  int hasMeshObs = 0;
  int hasNormalObs = 0;
  for (x = 0; x < fullList.count; x++) {
    if (fullList.list[x]->getType() == MeshObstacle::getClassName()) {
      hasMeshObs = 1;
    } else {
      hasNormalObs = 1;
    }
  }
  int color = hasNormalObs + (2 * hasMeshObs);

  // draw Z-normal squares
  for (z = 0; z < 2; z++) {
    for (c = 0; c < 4; c++) {
      x = ((c + 0) % 4) / 2;
      y = ((c + 1) % 4) / 2;
      points[c][0] = extents[x][0];
      points[c][1] = extents[y][1];
      points[c][2] = extents[z][2];
    }
    memcpy (points[4], points[0], sizeof (points[4]));
    drawLinesFunc (5, points, color);
  }

  // draw the corner edges
  for (c = 0; c < 4; c++) {
    x = ((c + 0) % 4) / 2;
    y = ((c + 1) % 4) / 2;
    for (z = 0; z < 2; z++) {
      points[z][0] = extents[x][0];
      points[z][1] = extents[y][1];
      points[z][2] = extents[z][2];
    }
    drawLinesFunc (2, points, color);
  }

  // draw the kids
  for (c = 0; c < childCount; c++) {
    children[c]->draw (drawLinesFunc);
  }

  return;
}


// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
