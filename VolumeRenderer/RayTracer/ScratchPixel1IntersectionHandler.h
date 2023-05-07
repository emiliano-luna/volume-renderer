#pragma once
#ifndef VOLUMERENDERER_SCRATCH1INTERSECTIONHANDLER
#define VOLUMERENDERER_SCRATCH1INTERSECTIONHANDLER

#include "BaseIntersectionHandler.h"
#include "Utils\Utils.h"

class ScratchPixel1IntersectionHandler :
    public BaseIntersectionHandler
{
public:
    virtual Vec3f HandleIntersection(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
};

#endif // !VOLUMERENDERER_SCRATCH1INTERSECTIONHANDLER