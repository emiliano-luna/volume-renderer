#pragma once
#ifndef VOLUMERENDERER_DIRECTIONSAMPLER
#define VOLUMERENDERER_DIRECTIONSAMPLER

#include "Types.h"
#include "Utils.h"
#include "ONB.h"

class DirectionSampler
{
public:
	static Vec3f getCosineDistributionRebound(Vec3f normal);
};

#endif

