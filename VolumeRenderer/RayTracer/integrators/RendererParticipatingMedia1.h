#define NOMINMAX

#pragma once
#ifndef VOLUMERENDERER_RENDERERPARTICIPATINGMEDIA1
#define VOLUMERENDERER_RENDERERPARTICIPATINGMEDIA1

#include <Windows.h>
#include "Process.h"
#include "../Utils\Types.h"
#include "../Utils\Utils.h"
#include "FreeImage.h"
#include <vector>
#include <rtcore.h>
#include <stdio.h>
#include <math.h>
#include "BaseIntegrator.h"

class RendererParticipatingMedia1 : public BaseIntegrator
{
public:			
	virtual Vec3f castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
	void handleIntersection(HandleIntersectionData* data, float absorptionChance, float emissionChance, nanovdb::DefaultReadAccessor<float> gridAccesor, float sigmaMax);
	float directLightningRayMarch(HandleIntersectionData* data, nanovdb::DefaultReadAccessor<float> gridAccesor, float maxStepSize, float sigmaMax);
};

#endif