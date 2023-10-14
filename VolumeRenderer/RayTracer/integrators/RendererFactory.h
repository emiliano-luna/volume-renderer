#pragma once
#ifndef VOLUMERENDERER_RENDERERFACTORY
#define VOLUMERENDERER_RENDERERFACTORY

#include <string>
#include "BaseRenderer.h"

class RendererFactory
{
public:
	static BaseRenderer* GetRenderer(std::string name);
};
#endif 
