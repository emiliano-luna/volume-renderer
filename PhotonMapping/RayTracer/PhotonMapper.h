#pragma once
#include "Utils.h"
#include "Types.h"
#include "SceneLoader.h"
#include "nanonflann\nanoflann.hpp"
#include "nanonflann\utils.h"
#include "ONB.h"
#include "Options.h"

class PhotonMapper
{
private:
	RTCRayHit static castRay(RTCScene scene,
		Vec3f origin,
		Vec3f direction);
	std::pair<PointCloud, std::vector<PhotonData>>* GeneratePhotons(SceneInfo* sceneInfo, PhotonMapOptions* options);
	void static GeneratePhotonRay(Vec3f origin, Vec3f direction, Vec3f photonColor, unsigned reboundLimit, PointCloud* cloud, std::vector<PhotonData>* data, SceneInfo* sceneInfo);
	Vec3f static getRebound(Vec3f normal);
public:	
	//TO DO: Guardar a archivo
	std::pair<PointCloud, std::vector<PhotonData>>* Generate(PhotonMapOptions* options);
};

