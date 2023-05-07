#pragma once
#ifndef VOLUMERENDERER_DIFFUSEINTERSECTIONHANDLER
#define VOLUMERENDERER_DIFFUSEINTERSECTIONHANDLER

#include "BaseIntersectionHandler.h"
#include "Utils/Utils.h"
#include "Utils/DirectionSampler.h"
#include "Utils/DirectLightSampler.h"
#include "Renderer.h"

class DiffuseIntersectionHandler :
    public BaseIntersectionHandler
{
public:
    virtual Vec3f HandleIntersection(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
};
#endif

