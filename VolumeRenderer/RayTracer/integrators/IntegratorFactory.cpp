#include "IntegratorFactory.h"
#include "BaseIntegrator.h"
#include "IntegratorDensitySampling.h"
#include "IntegratorHomogeneousRayMarcherNEE.h"
#include "IntegratorHomogeneousRayMarcherImproved.h"
#include "IntegratorHeterogeneousPerlinNoise.h"
#include "IntegratorNanoVDBSimple.h"
#include "RendererNanoVDBEmission.h"
#include "RendererParticipatingMedia1.h"
#include "RendererParticipatingMediaTransmission.h"
#include "RendererDeltaTracking2.h"
#include "RendererPBRTSimple.h"
#include "RendererPBRTVolume.h"

BaseIntegrator* IntegratorFactory::GetIntegrator(std::string name)
{
    if (name._Equal("densitySampling")) return new IntegratorDensitySampling();
    if (name._Equal("homogeneousRayMarcherNEE")) return new IntegratorHomogeneousRayMarcherNEE();
    if (name._Equal("homogeneousRayMarcherImproved")) return new IntegratorHomogeneousRayMarcherImproved();
    if (name._Equal("heterogeneousPerlinNoise")) return new IntegratorHeterogeneousPerlinNoise();
    if (name._Equal("nanoVDBSimple")) return new IntegratorNanoVDBSimple();
    if (name._Equal("nanoVDBEmission")) return new RendererNanoVDBEmission();
    if (name._Equal("participatingMedia1")) return new RendererParticipatingMedia1();
    if (name._Equal("participatingMediaTransmission")) return new RendererParticipatingMediaTransmission();
    if (name._Equal("deltaTracking2")) return new RendererDeltaTracking2();
    if (name._Equal("pbrtSimple")) return new RendererPBRTSimple();
    if (name._Equal("pbrtVolume")) return new RendererPBRTVolume();
        //return new RendererScratchPixel4();

    return nullptr;
}
