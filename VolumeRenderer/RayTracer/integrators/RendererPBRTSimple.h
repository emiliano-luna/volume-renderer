#define NOMINMAX

#pragma once
#ifndef VOLUMERENDERER_RENDERERPBRTSIMPLE
#define VOLUMERENDERER_RENDERERPBRTSIMPLE

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
#include "BaseRenderer.h"

/// <summary>
/// Based on SimpleVolPathIntegrator from PBRT 4.0 - delta tracking (no ray transmission) without direct lighting added to resulting image
/// </summary>
class RendererPBRTSimple : public BaseRenderer
{
public:			
	virtual Vec3f castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
	Vec3f getEmission(HandleIntersectionData* data, float emissionWeight);
};

#endif