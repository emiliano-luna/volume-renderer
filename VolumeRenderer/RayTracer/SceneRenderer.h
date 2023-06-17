#pragma once
#include <rtcore.h>
#include <stdio.h>
#include <math.h>
#include "tinyobjloader\tiny_obj_loader.h"
#include "Utils\Types.h"
#include "SceneLoader.h"
#include "nanonflann\utils.h"

class SceneRenderer
{
public:
	void RenderScene(Options options);
};

