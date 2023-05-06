#pragma once
#ifndef VOLUMERENDERER_DIRECTLIGHTSAMPLER
#define VOLUMERENDERER_DIRECTLIGHTSAMPLER

#include "Types.h"
#include "../BaseIntersectionHandler.h"
#include "Utils.h"

class DirectLightSampler
{
public:
	static Vec3f Sample(Vec3f origin, SceneInfo* sceneInfo);
private:
	static bool SampleLight(Vec3f origin, Vec3f direction, Vec3f& irradiance, PointLight light, SceneInfo* sceneInfo);
};

#endif
