#pragma once
#ifndef VOLUMERENDERER_PHASEFUNCTION
#define VOLUMERENDERER_PHASEFUNCTION

#include "Types.h"

class PhaseFunction
{
public:
	static float heyney_greenstein(const float& g, const float& cos_theta);
};

#endif