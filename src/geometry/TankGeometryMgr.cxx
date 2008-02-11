/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// bzflag common headers
#include "common.h"
#include "global.h"
#include "TextUtils.h"

// interface header
#include "TankGeometryMgr.h"

// system headers
#include <stdlib.h>
#include <math.h>
#include <string>
#include <fstream>

// common implementation headers
#include "SceneRenderer.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "OpenGLGState.h"

#include "PlatformFactory.h"
#include "BzfMedia.h"


// use the namespaces
using namespace TankGeometryMgr;
using namespace TankGeometryEnums;
using namespace TankGeometryUtils;


// Local Variables
// ---------------

// the display lists
static GLuint displayLists[LastTankShadow][LastTankLOD]
			  [LastTankSize][LastTankPart];

// triangle counds
static int partTriangles[LastTankShadow][LastTankLOD]
			[LastTankSize][LastTankPart];

// the scaling factors
static GLfloat scaleFactors[LastTankSize][3] = {
  {1.0f, 1.0f, 1.0f},   // Normal
  {1.0f, 1.0f, 1.0f},   // Obese
  {1.0f, 1.0f, 1.0f},   // Tiny
  {1.0f, 0.001f, 1.0f}, // Narrow
  {1.0f, 1.0f, 1.0f}    // Thief
};
// the current scaling factors
const float* TankGeometryUtils::currentScaleFactor = scaleFactors[Normal];

// the current shadow mode (used to remove glNormal3f and glTexcoord2f calls)
TankShadow TankGeometryUtils::shadowMode = ShadowOn;

// arrays of functions to avoid large switch statements
typedef int (*partFunction)(void);
static const partFunction partFunctions[LastTankLOD][BasicTankParts] = {
  { buildLowBody,
    buildLowBarrel,
    buildLowTurret,
    buildLowLCasing,
    buildLowRCasing
  },
  { buildMedBody,
    buildMedBarrel,
    buildMedTurret,
    buildMedLCasing,
    buildMedRCasing
  },
  { buildHighBody,
    buildHighBarrel,
    buildHighTurret,
    buildHighLCasing,
    buildHighRCasing
  }
};


// Local Function Prototypes
// -------------------------

static void setupScales();
static void freeContext(void *data);
static void initContext(void *data);
static void bzdbCallback(const std::string& str, void *data);


/****************************************************************************/

// TankGeometryMgr Functions
// -------------------------


void TankGeometryMgr::init()
{
  // initialize the lists to invalid
  for (int shadow = 0; shadow < LastTankShadow; shadow++) {
    for (int lod = 0; lod < LastTankLOD; lod++) {
      for (int size = 0; size < LastTankSize; size++) {
	for (int part = 0; part < LastTankPart; part++) {
	  displayLists[shadow][lod][size][part] = INVALID_GL_LIST_ID;
	  partTriangles[shadow][lod][size][part] = 0;
	}
      }
    }
  }

  // install the BZDB callbacks
  // This MUST be done after BZDB has been initialized in main()
  BZDB.addCallback (StateDatabase::BZDB_OBESEFACTOR, bzdbCallback, NULL);
  BZDB.addCallback (StateDatabase::BZDB_TINYFACTOR, bzdbCallback, NULL);
  BZDB.addCallback (StateDatabase::BZDB_THIEFTINYFACTOR, bzdbCallback, NULL);
  BZDB.addCallback ("animatedTreads", bzdbCallback, NULL);

  // install the context initializer
  OpenGLGState::registerContextInitializer (freeContext, initContext, NULL);

  // setup the scaleFactors
  setupScales();

  return;
}


void TankGeometryMgr::kill()
{
  // remove the BZDB callbacks
  BZDB.removeCallback (StateDatabase::BZDB_OBESEFACTOR, bzdbCallback, NULL);
  BZDB.removeCallback (StateDatabase::BZDB_TINYFACTOR, bzdbCallback, NULL);
  BZDB.removeCallback (StateDatabase::BZDB_THIEFTINYFACTOR, bzdbCallback, NULL);
  BZDB.removeCallback ("animatedTreads", bzdbCallback, NULL);

  // remove the context initializer callback
  OpenGLGState::unregisterContextInitializer(freeContext, initContext, NULL);

  return;
}


void TankGeometryMgr::deleteLists()
{
  // delete the lists that have been aquired
  for (int shadow = 0; shadow < LastTankShadow; shadow++) {
    for (int lod = 0; lod < LastTankLOD; lod++) {
      for (int size = 0; size < LastTankSize; size++) {
	for (int part = 0; part < LastTankPart; part++) {
	  GLuint& list = displayLists[shadow][lod][size][part];
	  if (list != INVALID_GL_LIST_ID) {
	    glDeleteLists(list, 1);
	    list = INVALID_GL_LIST_ID;
	  }
	}
      }
    }
  }
  return;
}


void TankGeometryMgr::buildLists()
{
  // setup the tread style
  setTreadStyle(BZDB.evalInt("treadStyle"));

  // setup the scale factors
  setupScales();
  currentScaleFactor = scaleFactors[Normal];
  const bool animated = BZDBCache::animatedTreads;

  // setup the quality level
  const int divisionLevels[4][2] = { // wheel divs, tread divs
    {4, 4},   // low
    {8, 16},  // med
    {12, 24}, // high
    {16, 32}  // experimental
  };
  int quality = RENDERER.useQuality();
  if (quality < _LOW_QUALITY) {
    quality = _LOW_QUALITY;
  } else if (quality > _EXPERIMENTAL_QUALITY) {
    quality = _EXPERIMENTAL_QUALITY;
  }
  int wheelDivs = divisionLevels[quality][0];
  int treadDivs = divisionLevels[quality][1];

  for (int shadow = 0; shadow < LastTankShadow; shadow++) {
    for (int lod = 0; lod < LastTankLOD; lod++) {
      for (int size = 0; size < LastTankSize; size++) {

	// only do the basics, unless we're making an animated tank
	int lastPart = BasicTankParts;
	if (animated) {
	  lastPart = HighTankParts;
	}

	// set the shadow mode for the doNormal3f() and doTexcoord2f() calls
	shadowMode = (TankShadow) shadow;

	for (int part = 0; part < lastPart; part++) {

	  GLuint& list = displayLists[shadow][lod][size][part];
	  int& count = partTriangles[shadow][lod][size][part];

	  // get a new OpenGL display list
	  list = glGenLists(1);
	  glNewList(list, GL_COMPILE);

	  // setup the scale factor
	  currentScaleFactor = scaleFactors[size];

	  if ((part <= Turret) || (!animated)) {
	    // the basic parts
	    count = partFunctions[lod][part]();
	  } else {
	    // the animated parts
	    if (part == LeftCasing) {
	      count = buildHighLCasingAnim();
	    }
	    else if (part == RightCasing) {
	      count = buildHighRCasingAnim();
	    }
	    else if (part == LeftTread) {
	      count = buildHighLTread(treadDivs);
	    }
	    else if (part == RightTread) {
	      count = buildHighRTread(treadDivs);
	    }
	    else if ((part >= LeftWheel0) && (part <= LeftWheel3)) {
	      int wheel = part - LeftWheel0;
	      count = buildHighLWheel(wheel, (float)wheel * (float)(M_PI / 2.0),
				      wheelDivs);
	    }
	    else if ((part >= RightWheel0) && (part <= RightWheel3)) {
	      int wheel = part - RightWheel0;
	      count = buildHighRWheel(wheel, (float)wheel * (float)(M_PI / 2.0),
				      wheelDivs);
	    }
	  }

	  // end of the list
	  glEndList();

	} // part
      } // size
    } // lod
  } // shadow

  return;
}


GLuint TankGeometryMgr::getPartList(TankGeometryEnums::TankShadow shadow,
				    TankGeometryEnums::TankPart part,
				    TankGeometryEnums::TankSize size,
				    TankGeometryEnums::TankLOD lod)
{
  return displayLists[shadow][lod][size][part];
}


int TankGeometryMgr::getPartTriangleCount(TankGeometryEnums::TankShadow sh,
					  TankGeometryEnums::TankPart part,
					  TankGeometryEnums::TankSize size,
					  TankGeometryEnums::TankLOD lod)
{
  return partTriangles[sh][lod][size][part];
}


const float* TankGeometryMgr::getScaleFactor(TankSize size)
{
  return scaleFactors[size];
}
class OBJVert
{
public:
  float x,y,z;

  OBJVert()
  {
    x = y = z = 0;
  }

  void glVertex ( void ) const
  {
    glVertex3f(x,y,z);
  }

  void glNormal ( void ) const
  {
    glNormal3f(x,y,z);
  }

  void glTexCoord ( void ) const
  {
    glTexCoord2f(x,y);
  }

  void read3 ( const char* t )
  {
    sscanf(t,"%f %f %f",&x,&y,&z);
  }

  void read2 ( const char* t )
  {
    sscanf(t,"%f %f",&x,&y);
  }
};

class OBJFace
{
public:
  std::vector<size_t> verts;
  std::vector<size_t> norms;
  std::vector<size_t> uvs;

  void draw ( const std::vector<OBJVert> &vertList, const std::vector<OBJVert> &normList, const std::vector<OBJVert> &uvList )
  {
    if (verts.size() == 3)
    {
      glBegin(GL_TRIANGLES);
    }
    else
    {
      glBegin(GL_POLYGON);
    }

    for ( size_t i = 0; i < verts.size(); i++ )
    {
      if ( verts[i] < vertList.size() )
	vertList[verts[i]].glVertex();
      if ( i < norms.size() && norms[i] < normList.size() )
	normList[norms[i]].glNormal();
      if ( i < uvs.size() &&  uvs[i] < uvList.size() )
	uvList[uvs[i]].glTexCoord();
    }

    glEnd();
  }
};

class OBJModel
{
public:
  std::vector<OBJFace> faces;

  std::vector<OBJVert>	vertList,normList,uvList;

  int draw ( void )
  {
    for ( size_t i = 0; i < faces.size(); i++ )
      faces[i].draw(vertList,normList,uvList);

    return (int) faces.size();
  }

  bool read ( const char *fileName )
  {
    std::ifstream ifs(fileName, std::ios::in);
    std::string line;
    while (ifs.good() && !ifs.eof())
    {
      std::getline(ifs, line);
      if ( line.size() )
      {
	// parse it
	switch(line[0])
	{
	case 'v':
	  if ( line.size() > 5 ) // there have to be enough characters for a full vert
	  {
	    OBJVert v;

	    switch (line[1])
	    {
	    case 'n':
	      v.read3(line.c_str()+2);
	      normList.push_back(v);
	      break;

	    case 't':
	      v.read2(line.c_str()+2);
	      uvList.push_back(v);
	      break;

	    default:
	      v.read3(line.c_str()+1);
	      vertList.push_back(v);
	      break;
	    }
	  }
	  break;

	case 'f':
	  {
	    std::vector<std::string> verts = TextUtils::tokenize(std::string(line.c_str()+2),std::string(" "));
	    OBJFace face;
	    for ( size_t v = 0; v < verts.size(); v++ )
	    {
	      std::vector<std::string> indexes = TextUtils::tokenize(verts[v],std::string("/"));
	      if (indexes.size())
	      {
		face.verts.push_back(atoi(indexes[0].c_str())-1);
		if ( indexes.size() > 1 && indexes[1].size() )
		  face.uvs.push_back(atoi(indexes[1].c_str())-1);
		else
		  face.uvs.push_back(0);

		if ( indexes.size() > 2 && indexes[2].size() )
		  face.norms.push_back(atoi(indexes[2].c_str())-1);
		else
		  face.norms.push_back(0);
	      }
	    }
	    if (face.verts.size())
	      faces.push_back(face);
	  }
	  break;
	}
      }
    }
    ifs.close();
    return true;
  }

};

std::map<std::string,OBJModel> modelMap;

std::string convertPathToNative ( const char* path )
{
#ifdef _WIN32
  return TextUtils::replace_all(std::string(path),std::string("/"),std::string("\\"));
#endif
  return std::string (path);
}

bool TankGeometryUtils::buildGeoFromObj ( const char* path, int &count  )
{ 
  std::string mediaPath = PlatformFactory::getMedia()->getMediaDirectory();
  mediaPath += convertPathToNative(path);
  count = 0;

  if (modelMap.find(mediaPath) != modelMap.end())
  {
    count = modelMap[mediaPath].draw();
    return count > 0;
  }  

  OBJModel model;
  if(model.read(mediaPath.c_str()))
  {
    modelMap[mediaPath] = model;
    count = model.draw();
  }
  return count > 0;
}

/****************************************************************************/

// Local Functions
// ---------------


static void bzdbCallback(const std::string& /*name*/, void * /*data*/)
{
  deleteLists();
  buildLists();
  return;
}


static void freeContext(void * /*data*/)
{
  // delete all of the lists
  deleteLists();
  return;
}


static void initContext(void * /*data*/)
{
  buildLists();
  return;
}


static void setupScales()
{
  float scale;

  scaleFactors[Normal][0] = BZDBCache::tankLength;
  scale = (float)atof(BZDB.getDefault(StateDatabase::BZDB_TANKLENGTH).c_str());
  scaleFactors[Normal][0] /= scale;

  scaleFactors[Normal][1] = BZDBCache::tankWidth;
  scale = (float)atof(BZDB.getDefault(StateDatabase::BZDB_TANKWIDTH).c_str());
  scaleFactors[Normal][1] /= scale;

  scaleFactors[Normal][2] = BZDBCache::tankHeight;
  scale = (float)atof(BZDB.getDefault(StateDatabase::BZDB_TANKHEIGHT).c_str());
  scaleFactors[Normal][2] /= scale;

  scale = BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
  scaleFactors[Obese][0] = scale * scaleFactors[Normal][0];
  scaleFactors[Obese][1] = scale * scaleFactors[Normal][1];
  scaleFactors[Obese][2] = scaleFactors[Normal][2];

  scale = BZDB.eval(StateDatabase::BZDB_TINYFACTOR);
  scaleFactors[Tiny][0] = scale * scaleFactors[Normal][0];
  scaleFactors[Tiny][1] = scale * scaleFactors[Normal][1];
  scaleFactors[Tiny][2] = scaleFactors[Normal][2];

  scale = BZDB.eval(StateDatabase::BZDB_THIEFTINYFACTOR);
  scaleFactors[Thief][0] = scale * scaleFactors[Normal][0];
  scaleFactors[Thief][1] = scale * scaleFactors[Normal][1];
  scaleFactors[Thief][2] = scaleFactors[Normal][2];

  scaleFactors[Narrow][0] = scaleFactors[Normal][0];
  scaleFactors[Narrow][1] = 0.001f;
  scaleFactors[Narrow][2] = scaleFactors[Normal][2];

  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
