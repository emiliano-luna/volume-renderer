#define NOMINMAX

#pragma once
#ifndef VOLUMERENDERER_RENDERERNANOVDBSIMPLE
#define VOLUMERENDERER_RENDERERNANOVDBSIMPLE

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

class RendererNanoVDBSimple : public BaseRenderer
{
public:			
	virtual Vec3f castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
	bool castLightRay(HandleIntersectionData* data);
};

#endif