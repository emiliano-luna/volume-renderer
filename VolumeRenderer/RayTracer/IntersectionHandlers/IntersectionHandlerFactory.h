#pragma once
#ifndef VOLUMERENDERER_INTERSECTIONHANDLERFACTORY
#define VOLUMERENDERER_INTERSECTIONHANDLERFACTORY

#include <string>
#include "BaseIntersectionHandler.h"
#include "BasicIntersectionHandler.h"
#include "ScratchPixel1IntersectionHandler.h"
#include "ScratchPixel2IntersectionHandler.h"
#include "DiffuseIntersectionHandler.h"

class IntersectionHandlerFactory
{
public:
	static BaseIntersectionHandler* GetIntersectionHandler(std::string name);
};
#endif 
