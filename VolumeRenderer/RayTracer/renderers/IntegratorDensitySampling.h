#define NOMINMAX

#pragma once
#ifndef VOLUMERENDERER_IntegratorDensitySampling
#define VOLUMERENDERER_IntegratorDensitySampling

#include <Windows.h>
#include "Process.h"
#include "../Utils\Types.h"
#include "../Utils\Utils.h"
#include "../IntersectionHandlers/BaseIntersectionHandler.h"
#include "../IntersectionHandlers/IntersectionHandlerFactory.h"
#include "FreeImage.h"
#include <vector>
#include <rtcore.h>
#include <stdio.h>
#include <math.h>
#include "../nanonflann\utils.h"
#include <chrono>
#include <thread>
#include "BaseRenderer.h"

class IntegratorDensitySampling : public BaseRenderer
{
public:			
	virtual Vec3f castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
	Vec3f handleIntersection(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
};

#endif