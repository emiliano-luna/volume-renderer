#pragma once
#ifndef VOLUMERENDERER_BASICINTERSECTIONHANDLER
#define VOLUMERENDERER_BASICINTERSECTIONHANDLER

#include "BaseIntersectionHandler.h"
#include "Utils\Utils.h"

class BasicIntersectionHandler :
    public BaseIntersectionHandler
{
public:
    virtual bool HandleIntersection(HandleIntersectionData* data, uint32_t depth);
};

#endif // !VOLUMERENDERER_BASICINTERSECTIONHANDLER