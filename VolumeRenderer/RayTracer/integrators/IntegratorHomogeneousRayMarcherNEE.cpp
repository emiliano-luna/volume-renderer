#include "IntegratorHomogeneousRayMarcherNEE.h"
#include "../Utils/DirectLightSampler.h"
#include "../Utils/DirectionSampler.h"
#include "../Utils/EmbreeHelper.h"

//Uses Embree for collision detection
//Aproximated Method - Forward ray marcher - Homogeneous Media - Next Event Estimation
//based on https://www.scratchapixel.com/lessons/3d-basic-rendering/volume-rendering-for-developers/ray-marching-algorithm.html

Vec3f IntegratorHomogeneousRayMarcherNEE::castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor)
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
	else if (data->throughput == 1.0f) {
		data->L_total_diffuse = data->options.backgroundColor;
	}

	return data->L_total_diffuse;
}

Vec3f IntegratorHomogeneousRayMarcherNEE::handleIntersection(HandleIntersectionData *data, uint32_t depth, uint32_t reboundFactor)
{
	auto material = data->sceneInfo->materials[data->sceneInfo->shapes[data->objectId].mesh.material_ids[0]];

	//participating media	
	if (material.dissolve > 0)
	{
		//exit intersection
		if (data->previousObjectId == data->objectId)
		{
			float transparency = 1;
			Vec3f result = Vec3f(0.0f);
			auto distance = data->tFar;

			Vec3f light_dir = Vec3f{ data->options.lightPosition.x, data->options.lightPosition.y, data->options.lightPosition.z };
			Vec3f light_color = Vec3f{ data->options.lightColor.x, data->options.lightColor.y, data->options.lightColor.z };

			auto sigma_a = data->options.sigma_a;

			float step_size = 0.2;
			int ns = std::ceil(distance / step_size);
			step_size = distance / ns;

			auto rayDirection = Utils::normalize(data->rayDirection);
			auto rayOrigin = data->rayOrigin;

			for (size_t n = 0; n < ns; n++)
			{
				float t = step_size * (n + 0.5f);
				Vec3f samplePosition = rayOrigin + rayDirection * t;

				//current sample transparency
				float sampleAttenuation = exp(-step_size * sigma_a);

				// attenuate volume object transparency by current sample transmission value
				transparency *= sampleAttenuation;

				data->rayDirection = light_dir;
				data->rayOrigin = samplePosition + data->rayDirection * 0.001;				
				// In-Scattering. Find the distance traveled by light through 
				// the volume to our sample point. Then apply Beer's law.							
				if (EmbreeHelper::castSingleRay(data)){
					auto distanceRayLightToExitInVolume = data->tFar;

					float light_attenuation = exp(-distanceRayLightToExitInVolume * sigma_a);
					// attenuate in-scattering contrib. by the transmission of all samples accumulated so far
					result += 
						transparency * 
						light_color * 
						light_attenuation * 
						step_size;
				}		
			}						

			// combine background color and volumetric object color
			return data->options.backgroundColor * transparency + result;
		}
		else {
			data->rayOrigin = data->hitPoint + data->rayDirection * 0.001;

			return castRay(data, depth, reboundFactor);
		}
	}

	//Si estoy intersecando el mismo objeto, lo ignoro
	if (data->previousObjectId == data->objectId)
	{
		data->rayOrigin = data->rayOrigin + data->rayDirection * 0.001;

		return castRay(data, depth + 1, reboundFactor);
	}	
}
