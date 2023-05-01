#include "IntersectionHandlerFactory.h"

BaseIntersectionHandler* IntersectionHandlerFactory::GetIntersectionHandler(std::string name)
{
    if (name._Equal("basic")) return new BasicIntersectionHandler();
    return nullptr;
}
