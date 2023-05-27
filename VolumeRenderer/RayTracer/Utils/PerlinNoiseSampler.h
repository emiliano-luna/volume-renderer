#pragma once
#ifndef VOLUMERENDERER_PERLINNOISESAMPLER
#define VOLUMERENDERER_PERLINNOISESAMPLER

#include "Types.h"

class PerlinNoiseSampler
{
private:
	PerlinNoiseSampler();
	static PerlinNoiseSampler* _instance;
public:
	static PerlinNoiseSampler* getInstance();
	float eval_density(const Vec3f& p);
};

#endif //VOLUMERENDERER_PERLINNOISESAMPLER

