#define NOMINMAX

#pragma once
#ifndef VOLUMERENDERER_INTEGRATORHETEROGENEOUSPERLINNOISE
#define VOLUMERENDERER_INTEGRATORHETEROGENEOUSPERLINNOISE

#include <Windows.h>
#include "Process.h"
#include "../Utils\Types.h"
#include "../Utils\Utils.h"
#include "../IntersectionHandlers/BaseIntersectionHandler.h"
#include "FreeImage.h"
#include <vector>
#include <rtcore.h>
#include <stdio.h>
#include <math.h>
#include "BaseIntegrator.h"

class IntegratorHeterogeneousPerlinNoise : public BaseIntegrator
{
public:			
	virtual Vec3f castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
	Vec3f handleIntersection(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
};

#endif