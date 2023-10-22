#define NOMINMAX

#pragma once
#ifndef VOLUMERENDERER_IntegratorNanoVDBEmission
#define VOLUMERENDERER_IntegratorNanoVDBEmission

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

class IntegratorNanoVDBEmission : public BaseIntegrator
{
public:			
	virtual Vec3f castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
	bool castLightRay(HandleIntersectionData* data);
};

#endif