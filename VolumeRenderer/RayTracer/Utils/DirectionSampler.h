#pragma once
#ifndef VOLUMERENDERER_DIRECTIONSAMPLER
#define VOLUMERENDERER_DIRECTIONSAMPLER

#include "Types.h"
#include "Utils.h"
#include "ONB.h"
#include "RandomGenerator.h"

class DirectionSampler
{
public:
	static Vec3f getCosineDistributionRebound(Vec3f normal, RandomGenerator* generator);
	static Vec3f sampleHenyeyGreenstein(float g, Vec3f direction, RandomGenerator* generator);
private:
	static ONB* onb;
};

#endif

