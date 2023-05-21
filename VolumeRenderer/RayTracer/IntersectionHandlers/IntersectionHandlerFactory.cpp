#include "IntersectionHandlerFactory.h"

BaseIntersectionHandler* IntersectionHandlerFactory::GetIntersectionHandler(std::string name)
{
    if (name._Equal("basic")) return new BasicIntersectionHandler();
    if (name._Equal("scratchPixel1")) return new ScratchPixel1IntersectionHandler();
    if (name._Equal("scratchPixel2")) return new ScratchPixel2IntersectionHandler();
    if (name._Equal("scratchPixel3")) return new ScratchPixel3IntersectionHandler();
    if (name._Equal("diffuse")) return new DiffuseIntersectionHandler();
    
    throw std::invalid_argument("intersection handler from xml not set or invalid.");
}
