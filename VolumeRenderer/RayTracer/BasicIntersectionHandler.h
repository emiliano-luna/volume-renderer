#pragma once
#ifndef VOLUMERENDERER_BASICINTERSECTIONHANDLER
#define VOLUMERENDERER_BASICINTERSECTIONHANDLER

#include "BaseIntersectionHandler.h"
#include "Utils\Utils.h"

class BasicIntersectionHandler :
    public BaseIntersectionHandler
{
public:
    virtual Vec3f HandleIntersection(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
};

#endif // !VOLUMERENDERER_BASICINTERSECTIONHANDLER