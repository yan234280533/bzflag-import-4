/* bzflag
* Copyright (c) 1993 - 2005 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
* EffectsRenderer:
*	Encapsulates rendering of effects ( spawn flashes, sparks, explosions, etc...)
*
*/

#ifndef	BZF_EFFECTS_RENDERER_H
#define	BZF_EFFECTS_RENDERER_H

#include "common.h"

/* system headers */
#include <string>
#include <vector>

/* common interface headers */
#include "bzfgl.h"
#include "Extents.h"
#include "OpenGLGState.h"
#include "SceneRenderer.h"

#include "Singleton.h"

class BasicEffect
{
public:
	BasicEffect();
	virtual ~BasicEffect(){};

	virtual void setPos ( const float *pos, const float *rot );
	virtual void setVel ( const float *vel );
	virtual void setTeam ( int team );
	virtual void setStartTime ( float time );

	virtual void freeContext(void){};
	virtual void rebuildContext(void){};

	virtual bool update ( float time );
	virtual void draw(const SceneRenderer &) {};

protected:

	float	position[3];
	float	rotation[3];
	float velocity[3];
	int		teamColor;
	float	startTime;
	float	lifetime;
	float	lastTime;
	float	deltaTime;
	float	age;
};

typedef std::vector<BasicEffect*>	tvEffectsList;

class StdSpawnEffect : public BasicEffect
{
public:
	StdSpawnEffect();
	virtual ~StdSpawnEffect();

	virtual bool update ( float time );
	virtual void draw ( const SceneRenderer& sr );

protected:
	int				texture;
	OpenGLGState	ringState;

	float			radius;
};

class ConeSpawnEffect : public StdSpawnEffect
{
public:
	virtual bool update ( float time );
	virtual void draw ( const SceneRenderer& sr );
};

class RingSpawnEffect : public StdSpawnEffect
{
public:
	virtual bool update ( float time );
	virtual void draw ( const SceneRenderer& sr );
};

class StdShotEffect : public BasicEffect
{
public:
	StdShotEffect();
	virtual ~StdShotEffect();

	virtual bool update ( float time );
	virtual void draw ( const SceneRenderer& sr );

protected:
	int				texture;
	OpenGLGState	ringState;

	float			radius;
};

class FlashShotEffect : public StdShotEffect
{
public:
	FlashShotEffect();

	virtual bool update ( float time );
	virtual void draw ( const SceneRenderer& sr );

private:
	float	     length;
};

class StdDeathEffect : public BasicEffect
{
public:
	StdDeathEffect();
	virtual ~StdDeathEffect();

	virtual bool update ( float time );
	virtual void draw ( const SceneRenderer& sr );

protected:
	int				texture;
	OpenGLGState	ringState;

	float			radius;
};

class EffectsRenderer : public Singleton<EffectsRenderer>
{
public:
	// called once to setup the effects system
	void init(void);

	// called to update the various effects
	void update(void);

	// called to draw all the current effects
	void draw(const SceneRenderer& sr);

	// called when the GL lists need to be deleted
	void freeContext(void);

	// called when the GL lists need to be remade
	void rebuildContext(void);

	// spawn flashes
	void addSpawnEffect ( int team, const float* pos );
	std::vector<std::string> getSpawnEffectTypes ( void );

	// shot flashes
	void addShotEffect ( int team, const float* pos, float rot, const float* vel = NULL );
	std::vector<std::string> getShotEffectTypes ( void );

	// death effects
	void addDeathEffect ( int team, const float* pos, float rot );
	std::vector<std::string> getDeathEffectTypes ( void );

protected:
	friend class Singleton<EffectsRenderer>;

protected:
	EffectsRenderer();
	~EffectsRenderer();

	tvEffectsList	effectsList;
};

#endif // BZF_EFFECTS_RENDERER_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

