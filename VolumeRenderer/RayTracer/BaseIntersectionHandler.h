#pragma once
#ifndef VOLUMERENDERER_BASEINTERSECTIONHANDLER
#define VOLUMERENDERER_BASEINTERSECTIONHANDLER

#include "Utils\Types.h"

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
    float tFar;

    //scratchpixel sample 1
    float transmissionRemaining;
    /// <summary>
    /// BRDF * cos(theta_i) / pdf
    /// </summary>
    Vec3f throughput;
    //diffuse
    Vec3f L_total_diffuse;
};

class BaseIntersectionHandler
{
public:
	virtual Vec3f HandleIntersection(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor) = 0;
};

#endif //VOLUMERENDERER_BASEINTERSECTIONHANDLER

