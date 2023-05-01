#pragma once
#ifndef VOLUMERENDERER_BASEINTERSECTIONHANDLER
#define VOLUMERENDERER_BASEINTERSECTIONHANDLER

#include "Types.h"

class HandleIntersectionData {
public:
    int previousObjectId;
    Vec3f previousHitPoint;
    int objectId;
    Vec3f hitPoint;
    Vec3f hitNormal;
    Vec3f rayOrigin;
    Vec3f rayDirection;
    SceneInfo* sceneInfo;
    Options options;
};

class BaseIntersectionHandler
{
public:
	virtual bool HandleIntersection(HandleIntersectionData* data, Vec3f& resultColor) = 0;
};

#endif //VOLUMERENDERER_BASEINTERSECTIONHANDLER
