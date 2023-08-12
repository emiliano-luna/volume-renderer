#pragma once
#ifndef VOLUMERENDERER_BASEINTERSECTIONHANDLER
#define VOLUMERENDERER_BASEINTERSECTIONHANDLER

#include "..\Utils\Types.h"
#include "..\nanovdb\util\Ray.h"
#include "..\Utils\RandomGenerator.h"

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
    /// <summary>
    /// True if the ray hit geometry
    /// </summary>
    bool rayHit;

    //scratchpixel sample 1
    /// <summary>
    /// BRDF * cos(theta_i) / pdf
    /// </summary>
    Vec3f throughput;
    //diffuse
    Vec3f L_total_diffuse;
    Vec3f radiance;
    float transmission;
    
    //participatingMediaDeltaTracking
    float rayWeight;
    int depthRemaining;
    //nanovdb::CoordBBox* bbox;
        
    nanovdb::Ray<float> iRay;
    RandomGenerator* randomGenerator;
    ThreadInfo* threadInfo;
};

//class BaseIntersectionHandler
//{
//public:
//	virtual Vec3f HandleIntersection(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor) = 0;
//};

#endif //VOLUMERENDERER_BASEINTERSECTIONHANDLER

