#include "RendererFactory.h"
#include "BaseRenderer.h"
#include "RendererScratchPixel4.h"
#include "RendererNanoVDBSimple.h"
#include "RendererNanoVDBEmission.h"
#include "RendererParticipatingMedia1.h"
#include "RendererParticipatingMediaTransmission.h"

BaseRenderer* RendererFactory::GetRenderer(std::string name)
{
    if (name._Equal("scratchPixel4")) return new RendererScratchPixel4();
    if (name._Equal("nanoVDBSimple")) return new RendererNanoVDBSimple();
    if (name._Equal("nanoVDBEmission")) return new RendererNanoVDBEmission();
    if (name._Equal("participatingMedia1")) return new RendererParticipatingMedia1();
    if (name._Equal("participatingMediaTransmission")) return new RendererParticipatingMediaTransmission();
    return new RendererScratchPixel4();
}
