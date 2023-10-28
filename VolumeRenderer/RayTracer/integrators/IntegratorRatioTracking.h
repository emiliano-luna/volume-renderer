#define NOMINMAX

#pragma once
#ifndef VOLUMERENDERER_IntegratorRatioTracking
#define VOLUMERENDERER_IntegratorRatioTracking

#include "../Utils\Types.h"
#include "../Utils\Utils.h"
#include "BaseIntegrator.h"

class IntegratorRatioTracking : public BaseIntegrator
{
public:			
	virtual Vec3f castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
	float directLightningRayMarch(HandleIntersectionData* data, float maxStepSize, float sigmaMax);
};

#endif