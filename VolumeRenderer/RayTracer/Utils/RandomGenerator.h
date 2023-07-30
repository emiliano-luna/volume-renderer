#ifndef RAYTRACER_RANDOMGENERATOR
#define RAYTRACER_RANDOMGENERATOR

#pragma once

#include "Types.h"
#include <ctime>    // For time()
#include <cstdlib>  // For srand() and rand()
#include <random>
#include <functional>

class RandomGenerator
{
private:
	RandomGenerator();
	std::mt19937* generator;
public:
	RandomGenerator(unsigned int seed);
	float getFloat(float min, float max);
};

#endif // !1