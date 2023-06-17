#include "RendererFactory.h"
#include "BaseRenderer.h"
#include "RendererScratchPixel4.h"
#include "RendererNanoVDBSimple.h"

BaseRenderer* RendererFactory::GetRenderer(std::string name)
{
    if (name._Equal("scratchPixel4")) return new RendererScratchPixel4();
    if (name._Equal("nanoVDBSimple")) return new RendererNanoVDBSimple();
    return new RendererScratchPixel4();
}
