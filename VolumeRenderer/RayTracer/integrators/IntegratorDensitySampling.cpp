#include "IntegratorDensitySampling.h"
#include "../Utils/EmbreeHelper.h"

//Uses Embree for collision detection
//Analytic Method - Density sampling - Homogeneous Media
//based on https://www.scratchapixel.com/lessons/3d-basic-rendering/volume-rendering-for-developers/intro-volume-rendering.html
Vec3f IntegratorDensitySampling::castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor)
{
	if (depth >= data->options.maxDepth) {
		return Vec3f(0.0f);
	}

	struct RTCRayQueryContext context;	rtcInitRayQueryContext(&context);
	struct RTCRayHit rayhit;
	rayhit.ray.org_x = data->rayOrigin.x; rayhit.ray.org_y = data->rayOrigin.y; rayhit.ray.org_z = data->rayOrigin.z;
	rayhit.ray.dir_x = data->rayDirection.x; rayhit.ray.dir_y = data->rayDirection.y; rayhit.ray.dir_z = data->rayDirection.z;
	rayhit.ray.tnear = 0; rayhit.ray.tfar = std::numeric_limits<float>::infinity();
	rayhit.ray.mask = -1; rayhit.ray.flags = 0;
	rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID; rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

	//Intersects a single ray with the scene
	rtcIntersect1(data->sceneInfo->scene, &rayhit);

	data->rayHit = rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID;
	if (data->rayHit)
	{
		data->previousObjectId = data->objectId;
		data->previousHitPoint = data->hitPoint;

		data->objectId = data->sceneInfo->primitives[rayhit.hit.primID];
		data->hitPoint = rayhit.ray.tfar * data->rayDirection + data->rayOrigin;

		data->hitNormal = Vec3f(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z);

		data->tFar = rayhit.ray.tfar;

		return handleIntersection(data, depth, reboundFactor);
	}

	//No diffuse hits
	return data->options.backgroundColor;
}

Vec3f IntegratorDensitySampling::handleIntersection(HandleIntersectionData *data, uint32_t depth, uint32_t reboundFactor)
{
	auto material = data->sceneInfo->materials[data->sceneInfo->shapes[data->objectId].mesh.material_ids[0]];

	//exit intersection
	if (data->previousObjectId == data->objectId)
	{
		auto distance = data->tFar;
		auto sigma_a = data->options.sigma_a;
		//for now we use the diffuse color of the object as the scattering color
		auto scatteringColor = Vec3f(material.diffuse[0], material.diffuse[1], material.diffuse[2]);
		auto transmission = exp(-distance * sigma_a);

		return data->options.backgroundColor * transmission + scatteringColor * (1 - transmission);
	}
	//start intersection
	else {
		//just bump the ray forward and trace again
		data->rayOrigin = data->hitPoint + data->rayDirection * 0.001;
		return castRay(data, depth, reboundFactor);
	}

	//Si estoy intersecando el mismo objeto, lo ignoro
	if (data->previousObjectId == data->objectId)
	{
		data->rayOrigin = data->rayOrigin + data->rayDirection * 0.001;

		return castRay(data, depth + 1, reboundFactor);
	}
}
