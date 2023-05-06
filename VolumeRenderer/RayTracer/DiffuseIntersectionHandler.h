#pragma once
#ifndef VOLUMERENDERER_DIFFUSEINTERSECTIONHANDLER
#define VOLUMERENDERER_DIFFUSEINTERSECTIONHANDLER

#include "BaseIntersectionHandler.h"
#include "Utils.h"

class DiffuseIntersectionHandler :
    public BaseIntersectionHandler
{
public:
    virtual bool HandleIntersection(HandleIntersectionData* data, Vec3f& resultColor);
};
#endif

