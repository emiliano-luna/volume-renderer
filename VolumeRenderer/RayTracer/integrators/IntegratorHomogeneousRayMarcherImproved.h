#pragma once
#ifndef VOLUMERENDERER_INTEGRATORHOMOGENEOUSRAYMARCHERIMPROVED
#define VOLUMERENDERER_INTEGRATORHOMOGENEOUSRAYMARCHERIMPROVED

#include "BaseRenderer.h"
#include "../Utils\Utils.h"
#include "../Utils/PhaseFunction.h"

/// <summary>
/// Scratchpixel - Improving forward ray marcher
/// https://www.scratchapixel.com/lessons/3d-basic-rendering/volume-rendering-for-developers/ray-marching-get-it-right.html
/// </summary>

class IntegratorHomogeneousRayMarcherImproved : public BaseRenderer
{
public:
    virtual Vec3f castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
    Vec3f handleIntersection(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor);
};

#endif // !VOLUMERENDERER_SCRATCH3INTERSECTIONHANDLER