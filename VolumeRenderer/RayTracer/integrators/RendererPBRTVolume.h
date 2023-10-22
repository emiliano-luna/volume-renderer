#define NOMINMAX

#pragma once
#ifndef VOLUMERENDERER_RENDERERPBRTVOLUME
#define VOLUMERENDERER_RENDERERPBRTVOLUME

#include "../Utils\Types.h"
#include "../Utils\Utils.h"
#include <vector>
#include <rtcore.h>
#include "BaseIntegrator.h"

/// <summary>
/// Based on VolPathIntegrator from PBRT 4.0
/// </summary>
class RendererPBRTVolume : public BaseIntegrator
{
public:			
	virtual Vec3f castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
	Vec3f getEmission(HandleIntersectionData* data, float emissionWeight);
	float directLightningRayMarch(HandleIntersectionData* data, float maxStepSize, float sigmaMax);
};

#endif