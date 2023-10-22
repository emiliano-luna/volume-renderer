#define NOMINMAX

#pragma once
#ifndef VOLUMERENDERER_RENDERERPARTICIPATINGMEDIATRANSMISSION
#define VOLUMERENDERER_RENDERERPARTICIPATINGMEDIATRANSMISSION

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

class RendererParticipatingMediaTransmission : public BaseIntegrator
{
public:			
	virtual Vec3f castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
	void handleIntersection(HandleIntersectionData* data, float sigma);
	float directLightningRayMarch(HandleIntersectionData* data, nanovdb::Ray<float> lightRay, nanovdb::DefaultReadAccessor<float> gridAccesor, float maxStepSize, float sigmaMax);
};

#endif