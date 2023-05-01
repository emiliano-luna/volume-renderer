#pragma once
#ifndef VOLUMERENDERER_SCRATCH1INTERSECTIONHANDLER
#define VOLUMERENDERER_SCRATCH1INTERSECTIONHANDLER

#include "BaseIntersectionHandler.h"
#include "Utils.h"

class ScratchPixel1IntersectionHandler :
    public BaseIntersectionHandler
{
public:
    virtual bool HandleIntersection(HandleIntersectionData* data, Vec3f& resultColor);
};

#endif // !VOLUMERENDERER_SCRATCH1INTERSECTIONHANDLER