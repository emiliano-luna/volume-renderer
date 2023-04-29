// MathLibrary.cpp : Defines the exported functions for the DLL.
#include "stdafx.h" // use stdafx.h in Visual Studio 2017 and earlier
//#include <utility>
//#include <limits.h>
#include "RayTracer.h"
#include "SceneRenderer.h"
#include "FileManager.h"
#include "Types.h"
#include <chrono>

void RenderScene()
{
	auto renderer = new SceneRenderer();

	auto configOptions = FileManager::GetRendererOptions();
	
	renderer->RenderScene(*configOptions);

	std::getchar();
}
