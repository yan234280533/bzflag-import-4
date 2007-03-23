/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "SceneBuilder.h"

// local implemenation headers
#include "ZSceneDatabase.h"
#include "BSPSceneDatabase.h"
#include "World.h"

// scene node implemenation headers
#include "MeshPolySceneNode.h"
#include "TankSceneNode.h"
#include "BoxSceneNodeGenerator.h"
#include "WallSceneNodeGenerator.h"
#include "MeshSceneNodeGenerator.h"
#include "BaseSceneNodeGenerator.h"
#include "PyramidSceneNodeGenerator.h"
#include "ObstacleSceneNodeGenerator.h"
#include "TeleporterSceneNodeGenerator.h"
#include "EighthDimShellNode.h"
#include "EighthDBoxSceneNode.h"
#include "EighthDPyrSceneNode.h"
#include "EighthDBaseSceneNode.h"

// common implementation headers
#include "StateDatabase.h"
#include "SceneRenderer.h"
#include "BZDBCache.h"
#include "TextureManager.h"
#include "ObstacleList.h"
#include "ObstacleMgr.h"


// uncomment for cheaper eighth dimension scene nodes
//#define SHELL_INSIDE_NODES


//
// SceneDatabaseBuilder
//

static const GLfloat	black[3] = { 0.0f, 0.0f, 0.0f };

const GLfloat		SceneDatabaseBuilder::boxColors[6][4] = {
				{ 0.75f, 0.25f, 0.25f, 1.0f },
				{ 0.63f, 0.25f, 0.25f, 1.0f },
				{ 0.75f, 0.25f, 0.25f, 1.0f },
				{ 0.75f, 0.375f, 0.375f, 1.0f },
				{ 0.875f, 0.5f, 0.5f, 1.0f },
				{ 0.275f, 0.2f, 0.2f, 1.0f }
			};
const GLfloat		SceneDatabaseBuilder::boxModulateColors[6][4] = {
				{ 0.75f, 0.75f, 0.75f, 1.0f },
				{ 0.63f, 0.63f, 0.63f, 1.0f },
				{ 0.75f, 0.75f, 0.75f, 1.0f },
				{ 0.69f, 0.69f, 0.69f, 1.0f },
				{ 0.875f, 0.875f, 0.875f, 1.0f },
				{ 0.375f, 0.375f, 0.375f, 1.0f }
			};
const GLfloat		SceneDatabaseBuilder::boxLightedColors[6][4] = {
				{ 0.75f, 0.25f, 0.25f, 1.0f },
				{ 0.75f, 0.25f, 0.25f, 1.0f },
				{ 0.75f, 0.25f, 0.25f, 1.0f },
				{ 0.75f, 0.25f, 0.25f, 1.0f },
				{ 0.875f, 0.5f, 0.5f, 1.0f },
				{ 0.875f, 0.5f, 0.5f, 1.0f }
			};
const GLfloat		SceneDatabaseBuilder::boxLightedModulateColors[6][4] = {
				{ 0.75f, 0.75f, 0.75f, 1.0f },
				{ 0.75f, 0.75f, 0.75f, 1.0f },
				{ 0.75f, 0.75f, 0.75f, 1.0f },
				{ 0.75f, 0.75f, 0.75f, 1.0f },
				{ 0.875f, 0.875f, 0.875f, 1.0f },
				{ 0.875f, 0.875f, 0.875f, 1.0f }
			};

const GLfloat		SceneDatabaseBuilder::teleporterColors[3][4] = {
				{ 1.0f, 0.875f, 0.0f, 1.0f },
				{ 0.9f, 0.8f, 0.0f, 1.0f },
				{ 0.0f, 0.0f, 0.0f, 0.5f }
			};
const GLfloat		SceneDatabaseBuilder::teleporterModulateColors[3][4] = {
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				{ 0.9f, 0.9f, 0.9f, 1.0f },
				{ 0.0f, 0.0f, 0.0f, 0.5f }
			};
const GLfloat		SceneDatabaseBuilder::teleporterLightedColors[3][4] = {
				{ 1.0f, 0.875f, 0.0f, 1.0f },
				{ 1.0f, 0.875f, 0.0f, 1.0f },
				{ 0.0f, 0.0f, 0.0f, 0.5f }
			};
const GLfloat		SceneDatabaseBuilder::teleporterLightedModulateColors[3][4] = {
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				{ 0.0f, 0.0f, 0.0f, 0.5f }
			};


SceneDatabaseBuilder::SceneDatabaseBuilder(const SceneRenderer* _renderer) :
				renderer(_renderer),
				wallMaterial(black, black, 0.0f),
				boxMaterial(black, black, 0.0f),
				pyramidMaterial(black, black, 0.0f),
				teleporterMaterial(black, black, 0.0f)
{
  // FIXME -- should get texture heights from resources

  TextureManager &tm = TextureManager::instance();


  // make styles -- first the outer wall
  int wallTexture = tm.getTextureID( "wall" );
  wallTexWidth = wallTexHeight = 10.0f;
  if (wallTexture>=0)
    wallTexWidth = tm.GetAspectRatio(wallTexture) * wallTexHeight;


  // make box styles
  int boxTexture = tm.getTextureID( "boxwall" );
  boxTexWidth = boxTexHeight = 0.2f * BZDB.eval(StateDatabase::BZDB_BOXHEIGHT);
  if (boxTexture>=0)
    boxTexWidth = tm.GetAspectRatio(boxTexture) * boxTexHeight;


  // lower maximum tank lod if lowdetail is true
  if (renderer->useQuality() == 0)
    TankSceneNode::setMaxLOD(2);

  engine = csQueryRegistry<iEngine>
    (csApplicationFramework::GetObjectRegistry());
  if (!engine)
    csApplicationFramework::ReportError("Failed to locate 3D Engine!");
}


SceneDatabaseBuilder::~SceneDatabaseBuilder()
{
  // do nothing
}


SceneDatabase* SceneDatabaseBuilder::make(const World* world)
{
  // We find the sector called "room".
  room = engine->FindSector("room");

  // set LOD flags
  const bool doLODs = BZDBCache::lighting && BZDBCache::zbuffer;
  wallLOD = baseLOD = boxLOD = pyramidLOD = teleporterLOD = doLODs;

  // pick type of database
  SceneDatabase* db;
  if (BZDBCache::zbuffer)
    db = new ZSceneDatabase;
  else
    db = new BSPSceneDatabase;
  // FIXME -- when making BSP tree, try several shuffles for best tree

  if (!world) {
    return db;
  }

  // free any prior inside nodes
  world->freeInsideNodes();


  // add nodes to database
  unsigned int i;

  const ObstacleList& walls = OBSTACLEMGR.getWalls();
  for (i = 0; i < walls.size(); i++) {
    addWall(db, *((WallObstacle*) walls[i]));
  }

  const ObstacleList& boxes = OBSTACLEMGR.getBoxes();
  for (i = 0; i < boxes.size(); i++) {
    addBox (db, *((BoxBuilding*) boxes[i]));
  }

  const ObstacleList& bases = OBSTACLEMGR.getBases();
  for (i = 0; i < bases.size(); i++) {
    addBase (db, *((BaseBuilding*) bases[i]));
  }

  const ObstacleList& pyramids = OBSTACLEMGR.getPyrs();
  for (i = 0; i < pyramids.size(); i++) {
    addPyramid (db, *((PyramidBuilding*) pyramids[i]));
  }

  const ObstacleList& teles = OBSTACLEMGR.getTeles();
  for (i = 0; i < teles.size(); i++) {
    addTeleporter (db, *((Teleporter*) teles[i]), world);
  }

  const ObstacleList& meshes = OBSTACLEMGR.getMeshes();
  for (i = 0; i < meshes.size(); i++) {
    addMesh (db, (MeshObstacle*) meshes[i]);
  }

  // add the water level node
  addWaterLevel(db, world);

  db->finalizeStatics();

  engine->Prepare();

  return db;
}


void SceneDatabaseBuilder::addWaterLevel(SceneDatabase* db,
					 const World* world)
{
  float plane[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
  const float level = world->getWaterLevel();
  plane[3] = -level;

  // don't draw it if it isn't active
  if (level < 0.0f) {
    return;
  }

  // setup the vertex and texture coordinates
  float size = BZDBCache::worldSize;
  GLfloat3Array v(4);
  GLfloat3Array n(0);
  GLfloat2Array t(4);
  v[0][0] = v[0][1] = v[1][1] = v[3][0] = -size/2.0f;
  v[1][0] = v[2][0] = v[2][1] = v[3][1] = +size/2.0f;
  v[0][2] = v[1][2] = v[2][2] = v[3][2] = level;
  t[0][0] = t[0][1] = t[1][1] = t[3][0] = 0.0f;
  t[1][0] = t[2][0] = t[2][1] = t[3][1] = 2.0f;

  // get the material
  const BzMaterial* mat = world->getWaterMaterial();
  const bool noRadar = mat->getNoRadar();
  const bool noShadow = mat->getNoShadow();

  MeshPolySceneNode* node =
    new MeshPolySceneNode(plane, noRadar, noShadow, v, n, t);

  // setup the material
  MeshSceneNodeGenerator::setupNodeMaterial(node, mat);

  db->addStaticNode(node, false);

  return;
}


void SceneDatabaseBuilder::addWall(SceneDatabase*, const WallObstacle& o)
{
  if (o.getHeight() <= 0.0f) {
    return;
  }

  iMaterialWrapper *sideMaterial = NULL;

  // try object, standard
  if (o.userTextures[0].size())
    sideMaterial = engine->FindMaterial(o.userTextures[0].c_str());
  if (sideMaterial == NULL)
    sideMaterial = engine->FindMaterial("wall");

  float        b = o.getBreadth();
  float        h = o.getHeight();
  const float *p = o.getPosition();
  float        r = o.getRotation();

  csRef<iMeshFactoryWrapper> wallFactory
    = engine->CreateMeshFactory("crystalspace.mesh.object.genmesh",
				NULL);
  csRef<iGeneralFactoryState> wallFactState
    = scfQueryInterface<iGeneralFactoryState>
    (wallFactory->GetMeshObjectFactory());

  wallFactState->SetVertexCount(4);

  wallFactState->GetVertices()[0].Set(0, 0, -b);
  wallFactState->GetVertices()[1].Set(0, 0,  b);
  wallFactState->GetVertices()[2].Set(0, h, -b);
  wallFactState->GetVertices()[3].Set(0, h,  b);

  wallFactState->GetTexels()[0].Set(0, 0);
  wallFactState->GetTexels()[1].Set(o.getBreadth() / wallTexWidth,
				    0);
  wallFactState->GetTexels()[2].Set(0,
				    o.getHeight() / wallTexHeight);
  wallFactState->GetTexels()[3].Set(o.getBreadth() / wallTexWidth,
				    o.getHeight() / wallTexHeight);

  wallFactState->SetTriangleCount(2);  
  wallFactState->GetTriangles()[ 0].Set(0, 3, 1);
  wallFactState->GetTriangles()[ 1].Set(0, 2, 3);

  wallFactState->CalculateNormals();

  csRef<iMeshWrapper> wallMesh = engine->CreateMeshWrapper(wallFactory,
							   "Wall");

  wallMesh->GetMeshObject()->SetMaterialWrapper(sideMaterial);
  csRef<iGeneralMeshState> meshstate = scfQueryInterface<iGeneralMeshState>
    (wallMesh->GetMeshObject());
  meshstate->SetLighting(true);
  iMovable *wallMove = wallMesh->GetMovable();
  wallMove->SetPosition(room, csVector3(p[0], 0, p[1]));
  wallMove->SetTransform(csYRotMatrix3(r));
  wallMove->UpdateMove();
}

void SceneDatabaseBuilder::addMesh(SceneDatabase* db, MeshObstacle* mesh)
{
  WallSceneNode* node;
  MeshSceneNodeGenerator* nodeGen = new MeshSceneNodeGenerator (mesh);

  while ((node = nodeGen->getNextNode(wallLOD))) {
    // make the inside node
    const bool ownTheNode = db->addStaticNode(node, true);
    // The BSP can split a MeshPolySceneNode and then delete it, which is
    // not good for the EighthDimShellNode's referencing scheme. If the
    // BSP would have split and then deleted this node, ownTheNode will
    // return true, and the EighthDimShellNode will then own the node.
    EighthDimShellNode* inode = new EighthDimShellNode(node, ownTheNode);
    mesh->addInsideSceneNode(inode);
  }
  delete nodeGen;
}


void SceneDatabaseBuilder::addBox(SceneDatabase* db, BoxBuilding& o)
{
  // this assumes boxes have six parts:  four sides, a roof, and a bottom.
  int part = 0;
  WallSceneNode* node;
  ObstacleSceneNodeGenerator* nodeGen = new BoxSceneNodeGenerator(&o);
  TextureManager &tm = TextureManager::instance();
  int    boxTexture = -1;
  bool  useColorTexture[2] = {false,false};

  // try object, standard, then default
  if (o.userTextures[0].size())
    boxTexture = tm.getTextureID(o.userTextures[0].c_str(),false);
  if (boxTexture < 0)
    boxTexture = tm.getTextureID(BZDB.get("boxWallTexture").c_str(),true);

  useColorTexture[0] = boxTexture >= 0;

  int boxTopTexture = -1;

  if (o.userTextures[1].size())
    boxTopTexture = tm.getTextureID(o.userTextures[1].c_str(),false);
  if (boxTopTexture < 0)
    boxTopTexture = tm.getTextureID(BZDB.get("boxTopTexture").c_str(),true);

  useColorTexture[1] = boxTopTexture >= 0;

  float texutureFactor = BZDB.eval("boxWallTexRepeat");
  if (renderer->useQuality() >= 3)
    texutureFactor = BZDB.eval("boxWallHighResTexRepeat");

  while ((node = ((part < 4) ? nodeGen->getNextNode(
				-texutureFactor*boxTexWidth,
				-texutureFactor*boxTexWidth, boxLOD) :
    // I'm using boxTexHeight for roof since textures are same
    // size and this number is available
				nodeGen->getNextNode(
				-boxTexHeight,
				-boxTexHeight, boxLOD)))) {
    node->setLightedModulateColor(boxLightedModulateColors[part]);
    node->setMaterial(boxMaterial);
    if (part < 4){
      node->setTexture(boxTexture);
    }
    else{
      node->setTexture(boxTopTexture);
    }

#ifdef SHELL_INSIDE_NODES
    const bool ownTheNode = db->addStaticNode(node, true);
    EighthDimShellNode* inode = new EighthDimShellNode(node, ownTheNode);
    o.addInsideSceneNode(inode);
#else
    db->addStaticNode(node, false);
#endif // SHELL_INSIDE_NODES

    part = (part + 1) % 6;
  }

#ifndef SHELL_INSIDE_NODES
  // add the inside node
  GLfloat obstacleSize[3];
  obstacleSize[0] = o.getWidth();
  obstacleSize[1] = o.getBreadth();
  obstacleSize[2] = o.getHeight();
  SceneNode* inode =
    new EighthDBoxSceneNode(o.getPosition(), obstacleSize, o.getRotation());
  o.addInsideSceneNode(inode);
#endif // SHELL_INSIDE_NODES

  delete nodeGen;

  float        w = o.getWidth();
  float        b = o.getBreadth();
  float        h = o.getHeight();
  const float *p = o.getPosition();
  float        r = o.getRotation();

  csRef<iMeshFactoryWrapper> boxFactory
    = engine->CreateMeshFactory("crystalspace.mesh.object.genmesh",
				NULL);
  csRef<iGeneralFactoryState> boxFactState
    = scfQueryInterface<iGeneralFactoryState>
    (boxFactory->GetMeshObjectFactory());

  boxFactState->SetVertexCount(8);

  boxFactState->GetVertices()[0].Set(-w, h, b);
  boxFactState->GetVertices()[1].Set(-w, h,-b);
  boxFactState->GetVertices()[2].Set( w, h,-b);
  boxFactState->GetVertices()[3].Set( w, h, b);
  boxFactState->GetVertices()[4].Set(-w, 0, b);
  boxFactState->GetVertices()[5].Set(-w, 0,-b);
  boxFactState->GetVertices()[6].Set( w, 0,-b);
  boxFactState->GetVertices()[7].Set( w, 0, b);

  boxFactState->GetTexels()[0].Set(0, 0);
  boxFactState->GetTexels()[1].Set(1, 0);
  boxFactState->GetTexels()[2].Set(0, 1);
  boxFactState->GetTexels()[3].Set(1, 1);
  boxFactState->GetTexels()[4].Set(1, 0);
  boxFactState->GetTexels()[5].Set(0, 1);
  boxFactState->GetTexels()[6].Set(1, 1);
  boxFactState->GetTexels()[7].Set(0, 0);

  boxFactState->SetTriangleCount(12);  
  boxFactState->GetTriangles()[ 0].Set(0, 3, 1);
  boxFactState->GetTriangles()[ 1].Set(3, 2, 1);
  boxFactState->GetTriangles()[ 2].Set(4, 5, 7);
  boxFactState->GetTriangles()[ 3].Set(5, 6, 7);
  boxFactState->GetTriangles()[ 4].Set(0, 4, 3);
  boxFactState->GetTriangles()[ 5].Set(4, 7, 3);
  boxFactState->GetTriangles()[ 6].Set(1, 6, 5);
  boxFactState->GetTriangles()[ 7].Set(1, 2, 6);
  boxFactState->GetTriangles()[ 8].Set(0, 1, 5);
  boxFactState->GetTriangles()[ 9].Set(0, 5, 4);
  boxFactState->GetTriangles()[10].Set(2, 3, 7);
  boxFactState->GetTriangles()[11].Set(2, 7, 6);

  boxFactState->CalculateNormals();

  csRef<iMeshWrapper> boxMesh = engine->CreateMeshWrapper(boxFactory, "Box");

  iMaterialWrapper *sideMaterial = engine->FindMaterial("boxwall");
  boxMesh->GetMeshObject()->SetMaterialWrapper(sideMaterial);
  csRef<iGeneralMeshState> meshstate = scfQueryInterface<iGeneralMeshState>
    (boxMesh->GetMeshObject());
  meshstate->SetLighting(true);
  iMovable *boxMove = boxMesh->GetMovable();
  boxMove->SetPosition(room, csVector3(p[0], p[2], p[1]));
  boxMove->SetTransform(csYRotMatrix3(r));
  boxMove->UpdateMove();
}


void SceneDatabaseBuilder::addPyramid(SceneDatabase*, PyramidBuilding& o)
{
  // Using boxTexHeight since it's (currently) the same and it's already available
  float textureFactor = BZDB.eval("pyrWallHighResTexRepeat");

  iMaterialWrapper *pyrMaterial = NULL;

  // try object, standard
  if (o.userTextures[0].size())
    pyrMaterial = engine->FindMaterial(o.userTextures[0].c_str());
  if (pyrMaterial == NULL)
    pyrMaterial = engine->FindMaterial(BZDB.get("pyrWallTexture").c_str());

  float        w = o.getWidth();
  float        b = o.getBreadth();
  float        h = o.getHeight();
  const float *p = o.getPosition();
  float        r = o.getRotation();

  const float textureSize  = textureFactor * boxTexHeight;
  const float vertexLength = sqrtf(w * w + b * b + h * h);

  csMatrix3    rotationMatrix;
  csVector3    positionVector(p[0], p[2], p[1]);
  if (o.getZFlip()) {
    rotationMatrix     = csYRotMatrix3(r) * csMatrix3(csZRotMatrix3(M_PI));
    positionVector[1] += h;
  } else {
    rotationMatrix     = csYRotMatrix3(r);
  }

  csRef<iMeshFactoryWrapper> pyrFactory
    = engine->CreateMeshFactory("crystalspace.mesh.object.genmesh",
				NULL);
  csRef<iGeneralFactoryState> pyrFactState
    = scfQueryInterface<iGeneralFactoryState>
    (pyrFactory->GetMeshObjectFactory());

  pyrFactState->SetVertexCount(16);
  pyrFactState->SetTriangleCount(6);

  pyrFactState->GetVertices()[0].Set( w,  0,  b);
  pyrFactState->GetVertices()[1].Set(-w,  0,  b);
  pyrFactState->GetVertices()[2].Set( 0,  h,  0);
  pyrFactState->GetTexels()[0].Set(0, 0);
  pyrFactState->GetTexels()[1].Set(2 * w / textureSize, 0);
  pyrFactState->GetTexels()[2].Set(0, vertexLength / textureSize);
  pyrFactState->GetTriangles()[0].Set(1, 0, 2);

  pyrFactState->GetVertices()[3].Set(-w,  0,  b);
  pyrFactState->GetVertices()[4].Set(-w,  0, -b);
  pyrFactState->GetVertices()[5].Set( 0,  h,  0);
  pyrFactState->GetTexels()[3].Set(0, 0);
  pyrFactState->GetTexels()[4].Set(2 * b / textureSize, 0);
  pyrFactState->GetTexels()[5].Set(0, vertexLength / textureSize);
  pyrFactState->GetTriangles()[1].Set(4, 3, 5);

  pyrFactState->GetVertices()[6].Set(-w,  0, -b);
  pyrFactState->GetVertices()[7].Set( w,  0, -b);
  pyrFactState->GetVertices()[8].Set( 0,  h,  0);
  pyrFactState->GetTexels()[6].Set(0, 0);
  pyrFactState->GetTexels()[7].Set(2 * w / textureSize, 0);
  pyrFactState->GetTexels()[8].Set(0, vertexLength / textureSize);
  pyrFactState->GetTriangles()[2].Set(7, 6, 8);

  pyrFactState->GetVertices()[ 9].Set( w,  0, -b);
  pyrFactState->GetVertices()[10].Set( w,  0,  b);
  pyrFactState->GetVertices()[11].Set( 0,  h,  0);
  pyrFactState->GetTexels()[ 9].Set(0, 0);
  pyrFactState->GetTexels()[10].Set(2 * b / textureSize, 0);
  pyrFactState->GetTexels()[11].Set(0, vertexLength / textureSize);
  pyrFactState->GetTriangles()[3].Set(10, 9, 11);

  pyrFactState->GetVertices()[12].Set( w,  0,  b);
  pyrFactState->GetVertices()[13].Set( w,  0, -b);
  pyrFactState->GetVertices()[14].Set(-w,  0,  b);
  pyrFactState->GetVertices()[15].Set(-w,  0, -b);
  pyrFactState->GetTexels()[12].Set(0, 0);
  pyrFactState->GetTexels()[13].Set(2 * b / textureSize, 0);
  pyrFactState->GetTexels()[14].Set(0, 2 * w / textureSize);
  pyrFactState->GetTexels()[15].Set(2 * b / textureSize,
				    2 * w / textureSize);
  pyrFactState->GetTriangles()[4].Set(13, 12, 14);
  pyrFactState->GetTriangles()[5].Set(15, 13, 14);

  pyrFactState->CalculateNormals();

  csRef<iMeshWrapper> pyrMesh = engine->CreateMeshWrapper(pyrFactory, "Box");

  pyrMesh->GetMeshObject()->SetMaterialWrapper(pyrMaterial);
  csRef<iGeneralMeshState> meshstate = scfQueryInterface<iGeneralMeshState>
    (pyrMesh->GetMeshObject());
  meshstate->SetLighting(true);
  iMovable *pyrMove = pyrMesh->GetMovable();
  pyrMove->SetPosition(room, positionVector);
  pyrMove->SetTransform(rotationMatrix);
  pyrMove->UpdateMove();
}

void SceneDatabaseBuilder::addBase(SceneDatabase *db, BaseBuilding &o)
{
  WallSceneNode* node;
  ObstacleSceneNodeGenerator* nodeGen = new BaseSceneNodeGenerator(&o);

  TextureManager &tm = TextureManager::instance();
  int   boxTexture = -1;

  bool  useColorTexture[2] = {false,false};

  // try object, standard, then default
  if (o.userTextures[0].size())
    boxTexture = tm.getTextureID(o.userTextures[0].c_str(),false);
  if (boxTexture < 0) {
    std::string teamBase = Team::getImagePrefix((TeamColor)o.getTeam());
    teamBase += BZDB.get("baseWallTexture");
    boxTexture = tm.getTextureID(teamBase.c_str(),false);
  }
  if (boxTexture < 0)
    boxTexture = tm.getTextureID( BZDB.get("boxWallTexture").c_str() );

  useColorTexture[0] = boxTexture >= 0;

  int   baseTopTexture = -1;

  if (o.userTextures[1].size())
    baseTopTexture = tm.getTextureID(o.userTextures[1].c_str(),false);
  if (baseTopTexture < 0) {
    std::string teamBase = Team::getImagePrefix((TeamColor)o.getTeam());
    teamBase += BZDB.get("baseTopTexture").c_str();
    baseTopTexture = tm.getTextureID(teamBase.c_str(),false);
  }
  if (baseTopTexture < 0)
    baseTopTexture = -1;
  else
    useColorTexture[1] = baseTopTexture >= 0;

  // this assumes bases have 6 parts - if they don't, it still works
  int part = 0;
  // repeat the textue once for the top and bottom, else use the old messed up way
  // There are 3 cases for the texture ordering:
  // 1. getNextNode() only returns the top texture
  // 2. getNextNode() returns the top texture(0), and the 4 sides(1-4)
  // 3. getNextNode() returns the top texture(0), and the 4 sides(1-4), and the bottom(5)
  while ((node = ( ((part % 5) == 0) ? nodeGen->getNextNode(1,1, boxLOD) :
				      nodeGen->getNextNode(o.getBreadth(),
							   o.getHeight(),
							   boxLOD)))) {
    if ((part % 5) != 0) {
      node->setLightedModulateColor(boxLightedModulateColors[part - 2]);
      node->setMaterial(boxMaterial);
      node->setTexture(boxTexture);
    }
    else{
      if (useColorTexture[1]) {  // only set the texture if we have one and are using it
	node->setTexture(baseTopTexture);
      }
    }
    part++;

#ifdef SHELL_INSIDE_NODES
    const bool ownTheNode = db->addStaticNode(node, true);
    EighthDimShellNode* inode = new EighthDimShellNode(node, ownTheNode);
    o.addInsideSceneNode(inode);
#else
    db->addStaticNode(node, false);
#endif // SHELL_INSIDE_NODES
  }

#ifndef SHELL_INSIDE_NODES
  // add the inside node
  GLfloat obstacleSize[3];
  obstacleSize[0] = o.getWidth();
  obstacleSize[1] = o.getBreadth();
  obstacleSize[2] = o.getHeight();
  SceneNode* inode = new
    EighthDBaseSceneNode(o.getPosition(), obstacleSize, o.getRotation());
  o.addInsideSceneNode(inode);
#endif // SHELL_INSIDE_NODES

  delete nodeGen;
}

void SceneDatabaseBuilder::addTeleporter(SceneDatabase* db,
					 const Teleporter& o,
					 const World* world)
{
  // this assumes teleporters have fourteen parts:  12 border sides, 2 faces
  int part = 0;
  WallSceneNode* node;
  ObstacleSceneNodeGenerator* nodeGen = new TeleporterSceneNodeGenerator(&o);

  TextureManager &tm = TextureManager::instance();
  int	     teleporterTexture = -1;

  bool  useColorTexture = false;

  // try object, standard, then default
  if (o.userTextures[0].size())
    teleporterTexture = tm.getTextureID(o.userTextures[0].c_str(),false);
  if (teleporterTexture < 0)
    teleporterTexture = tm.getTextureID(BZDB.get("cautionTexture").c_str(),true);

  useColorTexture = teleporterTexture >= 0;

  int numParts = o.isHorizontal() ? 18 : 14;

  while ((node = nodeGen->getNextNode(1.0, o.getHeight() / o.getBreadth(),
							teleporterLOD))) {
    if (o.isHorizontal ()) {
      if (part >= 0 && part <= 15) {
	node->setLightedModulateColor (teleporterLightedModulateColors[0]);
	node->setMaterial (teleporterMaterial);
	node->setTexture (teleporterTexture);
      }
    }
    else {
      if (part >= 0 && part <= 1) {
	node->setLightedModulateColor (teleporterLightedModulateColors[0]);
	node->setMaterial (teleporterMaterial);
	node->setTexture (teleporterTexture);
      }
      else if (part >= 2 && part <= 11) {
	node->setLightedModulateColor (teleporterLightedModulateColors[1]);
	node->setMaterial (teleporterMaterial);
	node->setTexture (teleporterTexture);
      }
    }

    db->addStaticNode(node, false);
    part = (part + 1) % numParts;
  }

  MeshPolySceneNode* linkNode;
  const BzMaterial* mat = world->getLinkMaterial();

  linkNode = MeshSceneNodeGenerator::getMeshPolySceneNode(o.getBackLink());
  MeshSceneNodeGenerator::setupNodeMaterial(linkNode, mat);
  db->addStaticNode(linkNode, false);

  linkNode = MeshSceneNodeGenerator::getMeshPolySceneNode(o.getFrontLink());
  MeshSceneNodeGenerator::setupNodeMaterial(linkNode, mat);
  db->addStaticNode(linkNode, false);

  delete nodeGen;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

