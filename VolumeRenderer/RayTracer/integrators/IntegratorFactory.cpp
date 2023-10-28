#include "IntegratorFactory.h"
#include "BaseIntegrator.h"
#include "IntegratorDensitySampling.h"
#include "IntegratorHomogeneousRayMarcherNEE.h"
#include "IntegratorHomogeneousRayMarcherImproved.h"
#include "IntegratorHeterogeneousPerlinNoise.h"
#include "IntegratorNanoVDBSimple.h"
#include "IntegratorNanoVDBEmission.h"
#include "IntegratorDeltaTracking.h"
#include "IntegratorRatioTracking.h"

BaseIntegrator* IntegratorFactory::GetIntegrator(std::string name)
{
    if (name._Equal("densitySampling")) return new IntegratorDensitySampling();
    if (name._Equal("homogeneousRayMarcherNEE")) return new IntegratorHomogeneousRayMarcherNEE();
    if (name._Equal("homogeneousRayMarcherImproved")) return new IntegratorHomogeneousRayMarcherImproved();
    if (name._Equal("heterogeneousPerlinNoise")) return new IntegratorHeterogeneousPerlinNoise();
    if (name._Equal("nanoVDBSimple")) return new IntegratorNanoVDBSimple();
    if (name._Equal("nanoVDBEmission")) return new IntegratorNanoVDBEmission();
    if (name._Equal("deltaTracking")) return new IntegratorDeltaTracking();
    if (name._Equal("ratioTracking")) return new IntegratorRatioTracking();

    return nullptr;
}
