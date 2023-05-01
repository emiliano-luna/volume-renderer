#include "IntersectionHandlerFactory.h"

BaseIntersectionHandler* IntersectionHandlerFactory::GetIntersectionHandler(std::string name)
{
    if (name._Equal("basic")) return new BasicIntersectionHandler();
    if (name._Equal("scratchPixel1")) return new ScratchPixel1IntersectionHandler();
    
    throw std::invalid_argument("intersection handler from xml not set or invalid.");
}
