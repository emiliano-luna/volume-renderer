#pragma once
#ifndef VOLUMERENDERER_BASICINTERSECTIONHANDLER
#define VOLUMERENDERER_BASICINTERSECTIONHANDLER

#include "BaseIntersectionHandler.h"
#include "Utils.h"

class BasicIntersectionHandler :
    public BaseIntersectionHandler
{
    virtual bool HandleIntersection(HandleIntersectionData* data, Vec3f& resultColor);
};

#endif // !VOLUMERENDERER_BASICINTERSECTIONHANDLER