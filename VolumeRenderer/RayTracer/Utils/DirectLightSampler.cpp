#include "DirectLightSampler.h"

/// <summary>
/// for now we assume with one light only with multiple vertices
/// </summary>
/// <param name="origin"></param>
/// <param name="sceneInfo"></param>
/// <returns></returns>
Vec3f DirectLightSampler::Sample(Vec3f origin, SceneInfo* sceneInfo)
{
	if (sceneInfo->lights.size() == 0)
		return Vec3f(0.0f);

	Vec3f irradiance;

	for (auto& light : sceneInfo->lights)
	{
		auto direction = light.position - origin;
		auto rayOrigin = origin + direction * 0.001f;

		if (SampleLight(rayOrigin, direction, irradiance, light, sceneInfo))
		{
			//https://geom.io/bakery/wiki/index.php?title=Point_Light_Attenuation
			//Bakery's "pysicall falloff"
			auto distance2 = Utils::distance2(light.position, origin);
			auto attenuationFactor = 1 / (distance2 + 1);

			return attenuationFactor * irradiance;
		}
	}

	return irradiance;
}

bool DirectLightSampler::SampleLight(Vec3f origin, Vec3f direction, Vec3f& irradiance, PointLight light, SceneInfo* sceneInfo) 
{
	irradiance = Vec3f(0.0f);

	struct RTCRayQueryContext context;
	rtcInitRayQueryContext(&context);

	struct RTCRayHit rayhit;
	rayhit.ray.org_x = origin.x; rayhit.ray.org_y = origin.y; rayhit.ray.org_z = origin.z;
	rayhit.ray.dir_x = direction.x; rayhit.ray.dir_y = direction.y; rayhit.ray.dir_z = direction.z;
	rayhit.ray.tnear = 0; rayhit.ray.tfar = std::numeric_limits<float>::infinity();
	rayhit.ray.mask = -1; rayhit.ray.flags = 0;
	rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID; rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

	//Intersects a single ray with the scene
	rtcIntersect1(sceneInfo->scene, &rayhit);

	//reached light
	if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
	{
		auto hitPoint = rayhit.ray.tfar * direction + origin;

		if (Utils::distance2(hitPoint, light.position) > 0.01)
			return false;

		auto material = sceneInfo->materials[sceneInfo->shapes[light.shapeIndex].mesh.material_ids[0]];

		irradiance = Vec3f(material.emission[0], material.emission[1], material.emission[2]);

		return true;
	}

	return false;
}
