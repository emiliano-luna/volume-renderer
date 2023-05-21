#include "PhaseFunction.h"

// the Henyey-Greenstein phase function
float PhaseFunction::heyney_greenstein(const float& g, const float& cos_theta)
{
	float denom = 1 + g * g - 2 * g * cos_theta;
	return 1 / (4 * M_PI) * (1 - g * g) / (denom * sqrtf(denom));
}