#include "RandomGenerator.h"
#include <random>

RandomGenerator::RandomGenerator(unsigned int seed)
{
	generator = new std::mt19937(seed);
}

float RandomGenerator::getFloat(float min, float max)
{
	std::uniform_real_distribution<float> distribution(min, max);

	return distribution(*generator);
}
