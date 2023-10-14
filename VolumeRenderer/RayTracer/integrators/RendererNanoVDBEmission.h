#define NOMINMAX

#pragma once
#ifndef VOLUMERENDERER_RENDERERNANOVDBEMISSION
#define VOLUMERENDERER_RENDERERNANOVDBEMISSION

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

class RendererNanoVDBEmission : public BaseRenderer
{
public:			
	virtual Vec3f castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
	bool castLightRay(HandleIntersectionData* data);
};

#endif