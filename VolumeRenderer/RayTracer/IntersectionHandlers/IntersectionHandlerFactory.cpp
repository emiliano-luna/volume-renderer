#include "IntersectionHandlerFactory.h"
#include "BasicIntersectionHandler.h"
#include "ScratchPixel1IntersectionHandler.h"
#include "ScratchPixel2IntersectionHandler.h"
#include "ScratchPixel3IntersectionHandler.h"
#include "ScratchPixel4IntersectionHandler.h"
#include "DiffuseIntersectionHandler.h"

BaseIntersectionHandler* IntersectionHandlerFactory::GetIntersectionHandler(std::string name)
{
    if (name._Equal("basic")) return new BasicIntersectionHandler();
    if (name._Equal("scratchPixel1")) return new ScratchPixel1IntersectionHandler();
    if (name._Equal("scratchPixel2")) return new ScratchPixel2IntersectionHandler();
    if (name._Equal("scratchPixel3")) return new ScratchPixel3IntersectionHandler();
    if (name._Equal("scratchPixel4")) return new ScratchPixel4IntersectionHandler();
    if (name._Equal("diffuse")) return new DiffuseIntersectionHandler();
    
    throw std::invalid_argument("intersection handler from xml not set or invalid.");
}
