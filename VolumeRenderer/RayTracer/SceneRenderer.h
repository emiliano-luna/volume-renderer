#pragma once
#include <rtcore.h>
#include <stdio.h>
#include <math.h>
#include "Renderer.h"
#include "tinyobjloader\tiny_obj_loader.h"
#include "Types.h"
#include "SceneLoader.h"
#include "nanonflann\utils.h"

class SceneRenderer
{
private:
	void castRay(RTCScene scene, float ox, float oy, float oz, float dx, float dy, float dz);	
public:
	void RenderScene(Options options);
};
