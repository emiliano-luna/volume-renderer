#define NOMINMAX

#pragma once
#ifndef VOLUMERENDERER_RENDERERPBRTVOLUME
#define VOLUMERENDERER_RENDERERPBRTVOLUME

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

class RendererPBRTVolume : public BaseRenderer
{
public:			
	virtual Vec3f castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
	Vec3f getEmission(HandleIntersectionData* data, float emissionWeight);
};

#endif