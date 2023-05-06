#pragma once
#ifndef VOLUMERENDERER_SCRATCH1INTERSECTIONHANDLER
#define VOLUMERENDERER_SCRATCH1INTERSECTIONHANDLER

#include "BaseIntersectionHandler.h"
#include "Utils\Utils.h"

class ScratchPixel1IntersectionHandler :
    public BaseIntersectionHandler
{
public:
    virtual bool HandleIntersection(HandleIntersectionData* data);
};

#endif // !VOLUMERENDERER_SCRATCH1INTERSECTIONHANDLER