#pragma once
#ifndef VOLUMERENDERER_EMBREEHELPER
#define VOLUMERENDERER_EMBREEHELPER

#include "../Utils/Types.h"
#include "../integrators/BaseIntegrator.h"

class EmbreeHelper
{
public:
	static bool castSingleRay(HandleIntersectionData* intersectionData);	
};

#endif