#pragma once
#ifndef VOLUMERENDERER_INTEGRATORHOMOGENEOUSRAYMARCHERNEE
#define VOLUMERENDERER_INTEGRATORHOMOGENEOUSRAYMARCHERNEE

#include "BaseIntegrator.h"
#include "../Utils\Utils.h"

class IntegratorHomogeneousRayMarcherNEE :
    public BaseIntegrator
{
public:
    virtual Vec3f castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
    Vec3f handleIntersection(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
};

#endif