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
    virtual bool HandleIntersection(HandleIntersectionData* data, uint32_t depth);
};
#endif

