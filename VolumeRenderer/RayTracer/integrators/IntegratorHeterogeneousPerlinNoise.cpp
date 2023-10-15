#include "IntegratorHeterogeneousPerlinNoise.h"
#include <random>
#include "../Utils/PerlinNoiseSampler.h"
#include "../Utils/EmbreeHelper.h"
#include "../Utils/PhaseFunction.h"
#include "../Utils/DirectLightSampler.h"
#include "../Utils/DirectionSampler.h"

//Uses Embree for collision detection
//Aproximated Method - Forward ray marcher - Heterogeneous Media with Perlin Noise
//based on https://www.scratchapixel.com/lessons/3d-basic-rendering/volume-rendering-for-developers/volume-rendering-3D-density-field.html
Vec3f IntegratorHeterogeneousPerlinNoise::castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor)
{
	if (depth >= data->options.maxDepth) {
		return Vec3f(0.0f);
	}

	struct RTCRayQueryContext context;
	rtcInitRayQueryContext(&context);

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

Vec3f IntegratorHeterogeneousPerlinNoise::handleIntersection(HandleIntersectionData *data, uint32_t depth, uint32_t reboundFactor)
{
	auto material = data->sceneInfo->materials[data->sceneInfo->shapes[data->objectId].mesh.material_ids[0]];

	//participating media	
	if (material.dissolve > 0)
	{
		//exit intersection
		if (data->previousObjectId == data->objectId)
		{
			auto distance = data->tFar;

			Vec3f light_dir = Vec3f{ data->options.lightPosition.x, data->options.lightPosition.y, data->options.lightPosition.z };
			Vec3f light_color = Vec3f{ data->options.lightColor.x, data->options.lightColor.y, data->options.lightColor.z };

			//absorption coefficient
			auto sigma_a = data->options.sigma_a;
			//scattering coefficient
			auto sigma_s = data->options.sigma_s;
			//extinction coefficient
			auto sigma_t = sigma_a + sigma_s;
			// heyney-greenstein asymmetry factor of the phase function
			float g = data->options.heyneyGreensteinG; 

			float step_size = 0.2;
			int ns = std::ceil(distance / step_size);

			//initialize transmission to 1 (fully transparent)
			float transparency = 1;
			//initialize volumetric sphere color to 0
			Vec3f result = Vec3f(0.0f);

			auto rayDirection = Utils::normalize(data->rayDirection);
			auto rayOrigin = data->rayOrigin;

			for (size_t n = 0; n < ns; n++)
			{
				static std::default_random_engine e;
				static std::uniform_real_distribution<> dis(0, 1);
				auto randomOffset = dis(e);

				//we use stochastic sampling to help with banding, even though it introduces noise
				//Stochastic sampling is a Monte Carlo technique in which we sample 
				//the function at appropriate non-uniformly spaced locations rather 
				//than at regularly spaced locations.
				float t = step_size * (n + randomOffset);

				Vec3f samplePosition = rayOrigin + rayDirection * t;
				//evaluate the density at the sample location (space varying density)
				auto density = PerlinNoiseSampler::getInstance()->eval_density(samplePosition);//eval_density(samplePosition);

				//current sample transparency
				float sampleAttenuation = exp(-step_size * density * sigma_t);

				// attenuate volume object transparency by current sample transmission value
				transparency *= sampleAttenuation;

				//prepare light ray
				data->rayDirection = light_dir;
				data->rayOrigin = samplePosition + data->rayDirection * 0.001;				
				// In-Scattering. Find the distance traveled by light through 
				// the volume to our sample point. Then apply Beer's law.							
				if (density > 0 &&
					EmbreeHelper::castSingleRay(data)){
					float tau = 0;
					auto distanceRayLightToExitInVolume = data->tFar;
					auto num_steps_light = std::ceil(distanceRayLightToExitInVolume / step_size);

					for (size_t nl = 0; nl < num_steps_light; ++nl) {
						float tLight = step_size * (nl + 0.5);
						Vec3f samplePosLight = samplePosition + tLight * light_dir;
						tau += PerlinNoiseSampler::getInstance()->eval_density(samplePosLight);
					}

					float cos_theta = Utils::dotProduct(rayDirection, light_dir);
					float light_attenuation = exp(-tau * step_size * sigma_t);
					result += 
						light_color *										// light color
						light_attenuation *									// light ray transmission value
						density *											// volume density at the sample position
						sigma_s *											// scattering coefficient
						PhaseFunction::heyney_greenstein(g, cos_theta) *	// phase function
						transparency *										// ray current transmission value
						step_size;											// dx in our Riemann sum

					// the greater the value the more often we will break out from the marching loop
					int d = 2;
					if (transparency < 1e-3) {
						// break
						if (data->randomGenerator->getFloat(0, 1) > 1.f / d)
							n = ns;
						// we continue but compensate
						else
							transparency *= d;
					}
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
