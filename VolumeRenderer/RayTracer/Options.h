#pragma once
#ifndef RAYTRACER_OPTIONS
#define RAYTRACER_OPTIONS

#include "Types.h"

class PhotonMapOptions {
public:
	unsigned int photonCount;
	std::string modelName;
	std::string fileName;
	Vec3f lightPos;
	Vec3f lightColor;
};
#endif