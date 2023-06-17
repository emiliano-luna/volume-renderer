#include "RendererFactory.h"
#include "BaseRenderer.h"

BaseRenderer* RendererFactory::GetRenderer(std::string name)
{
    /*if (name._Equal("basic")) 
    if (name._Equal("scratchPixel1")) return new ScratchPixel1IntersectionHandler();
    if (name._Equal("scratchPixel2")) return new ScratchPixel2IntersectionHandler();
    if (name._Equal("scratchPixel3")) return new ScratchPixel3IntersectionHandler();
    if (name._Equal("scratchPixel4")) return new ScratchPixel4IntersectionHandler();
    if (name._Equal("diffuse")) return new DiffuseIntersectionHandler();*/
    return new BaseRenderer();
}
