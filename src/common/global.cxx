/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#if (_WIN32)
// turn off bogus `this used in base member initialization list'
	#pragma warning(disable: 4786)
#endif


#include "global.h"

GlobalDBItem				globalDBItems[] = {
	{ "_angleTolerance",		"0.01",				false, StateDatabase::Locked},
	{ "_angularAd",			"1.5",				false, StateDatabase::Locked},
	{ "_boxHeight",			"6.0*_muzzleHeight",		false, StateDatabase::Locked},
	{ "_burrowAngularAd",		"0.55",				false, StateDatabase::Locked},
	{ "_burrowDepth",		"-1.32",			false, StateDatabase::Locked},
	{ "_burrowSpeedAd",		"0.80",				false, StateDatabase::Locked},
	{ "_explodeTime",		"5.0",				false, StateDatabase::Locked},
	{ "_flagAltitude",		"11.0",				false, StateDatabase::Locked},
	{ "_flagHeight",		"10.0",				false, StateDatabase::Locked},
	{ "_flagRadius",		"2.5",				false, StateDatabase::Locked},
	{ "_gMissileAng",		"0.628319",			false, StateDatabase::Locked},
	{ "_gMissileAdLife",		"0.95",				false, StateDatabase::Locked},
	{ "_gravity",			"-9.8",				false, StateDatabase::Locked},
	{ "_identifyRange",		"50.0",				false, StateDatabase::Locked},
	{ "_jumpVelocity",		"19.0",				false, StateDatabase::Locked},
	{ "_laserAdLife",		"0.1",				false, StateDatabase::Locked},
	{ "_laserAdRate",		"0.5",				false, StateDatabase::Locked},
	{ "_laserAdVel",		"1000.0",			false, StateDatabase::Locked},
	{ "_lockOnAngle",		"0.15",				false, StateDatabase::Locked},
	{ "_lRAdRate",			"0.5",				false, StateDatabase::Locked},
	{ "_maxFlagGrabs",		"4.0",				false, StateDatabase::Locked},
	{ "_maxLOD",			"32767.0",			false, StateDatabase::Locked},
	{ "_momentumAngAcc",		"1.0",				false, StateDatabase::Locked},
	{ "_momentumLinAcc",		"1.0",				false, StateDatabase::Locked},
	{ "_mGunAdLife",		"1.0 / _mGunAdRate",		false, StateDatabase::Locked},
	{ "_mGunAdRate",		"10.0",				false, StateDatabase::Locked},
	{ "_mGunAdVel",			"1.5",				false, StateDatabase::Locked},
	{ "_muzzleFront",		"_tankRadius + 0.1",		false, StateDatabase::Locked},
	{ "_muzzleHeight",		"1.57",				false, StateDatabase::Locked},
	{ "_notRespondingTime",		"3.5",				false, StateDatabase::Locked},
	{ "_obeseFactor",		"2.5",				false, StateDatabase::Locked},
	{ "_positionTolerance",		"0.01",				false, StateDatabase::Locked},
	{ "_pyrBase",			"4.0*_tankHeight",		false, StateDatabase::Locked},
	{ "_pyrHeight",			"5.0*_tankHeight",		false, StateDatabase::Locked},
	{ "_reloadTime",		"_shotRange / _shotSpeed",	false, StateDatabase::Locked},
	{ "_rFireAdLife",		"1.0 / _rFireAdRate",		false, StateDatabase::Locked},
	{ "_rFireAdRate",		"2.0",				false, StateDatabase::Locked},
	{ "_rFireAdVel",		"1.5",				false, StateDatabase::Locked},
	{ "_shieldFlight",		"2.7",				false, StateDatabase::Locked},
	{ "_shockAdLife",		"0.2",				false, StateDatabase::Locked},
	{ "_shockInRadius",		"_tankLength",			false, StateDatabase::Locked},
	{ "_shockOutRadius",		"60.0",				false, StateDatabase::Locked},
	{ "_shotRange",			"350.0",			false, StateDatabase::Locked},
	{ "_shotSpeed",			"100.0",			false, StateDatabase::Locked},
	{ "_srRadiusMult",		"2.0",				false, StateDatabase::Locked},
	{ "_tankAngVel",		"0.785398", 			false, StateDatabase::Locked},
	{ "_tankHeight",		"2.05",				false, StateDatabase::Locked},
	{ "_tankLength",		"6.0",				false, StateDatabase::Locked},
	{ "_tankRadius",		"0.72 * _tankLength",		false, StateDatabase::Locked},
	{ "_tankSpeed",			"25.0",				false, StateDatabase::Locked},
	{ "_tankWidth",			"2.8",				false, StateDatabase::Locked},
	{ "_targetingAngle",		"0.3",				false, StateDatabase::Locked},
	{ "_teleportTime",		"1.0",				false, StateDatabase::Locked},
	{ "_thiefAdLife",		"0.05",				false, StateDatabase::Locked},
	{ "_thiefAdRate",		"12.0",				false, StateDatabase::Locked},
	{ "_thiefAdShotVel",		"8.0",				false, StateDatabase::Locked},
	{ "_thiefTinyFactor",		"0.5",				false, StateDatabase::Locked},
	{ "_thiefVelAd",		"1.67",				false, StateDatabase::Locked},
	{ "_tinyFactor",		"0.4",				false, StateDatabase::Locked},
	{ "_updateThrottleRate",	"30.0",				false, StateDatabase::Locked},
	{ "_velocityAd",		"1.5",				false, StateDatabase::Locked},
	{ "_wallHeight",		"3.0*_tankHeight",		false, StateDatabase::Locked},
	{ "_wideAngleAng",		"1.745329", 			false, StateDatabase::Locked},
	{ "_worldSize",			"800.0",			false, StateDatabase::Locked},
};

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
