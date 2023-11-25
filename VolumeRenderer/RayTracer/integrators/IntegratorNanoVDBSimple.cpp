#include "IntegratorNanoVDBSimple.h"
#include "../nanovdb/NanoVDB.h"
#include "../nanovdb/util/Ray.h"
#include "../nanovdb/util/IO.h"
#include "../nanovdb/util/Primitives.h"
#include "../nanovdb/fog_example/common.h"
#include <random>
#include "../Utils/PhaseFunction.h"

Vec3f IntegratorNanoVDBSimple::castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor)
{
	auto rayDirection = Utils::normalize(data->rayDirection);
	auto densityAccesor = data->sceneInfo->densityGrid->tree().getAccessor();

	nanovdb::Vec3<float> rayEye = { data->rayOrigin.x, data->rayOrigin.y, data->rayOrigin.z };
	nanovdb::Vec3<float> rayDir = { data->rayDirection.x, data->rayDirection.y, data->rayDirection.z };
	// generate ray.
	nanovdb::Ray<float> wRay(rayEye, rayDir);
	// transform the ray to the grid's index-space.
	nanovdb::Ray<float> iRay = wRay.worldToIndexF(*data->sceneInfo->densityGrid);
	// clip to bounds.
	if (iRay.clip(data->sceneInfo->gridBoundingBox) == false) {
		return Vec3f(data->options.backgroundColor);
	}
	float density = 0.5f;
	float lightRayDensity = density * 0.5f;
	// integrate...
	const float step_size = 0.5f;
	//transparency
	float       transmittance = 1.0f;
	//initialize volumetric color to 0
	Vec3f result = Vec3f(0.0f);

	for (float t = iRay.t0(); t < iRay.t1(); t += step_size) {
		//cast light ray
		float sigma = densityAccesor.getValue(nanovdb::Coord::Floor(iRay(t))) * density;
		//current sample transparency
		float sampleAttenuation = exp(-step_size * sigma);
		// attenuate volume object transparency by current sample transmission value
		transmittance *= sampleAttenuation;
		//prepare light ray
		auto rayWorldPosition = data->sceneInfo->densityGrid->indexToWorldF(iRay(t));		

		data->rayDirection = data->options.lightPosition;
		data->rayOrigin = Vec3f(rayWorldPosition[0], rayWorldPosition[1], rayWorldPosition[2]);

		if (sigma > 0){	
			//in shadow
			if (castLightRay(data)) {
				float tau = 0;

				float light_step_size = step_size * 20.0f;
				auto num_steps_light = std::ceil(data->iRay.t1() / light_step_size);

				for (size_t nl = 0; nl < num_steps_light; ++nl) {
					float tLight = light_step_size * (nl + 0.5);
					tau += densityAccesor.getValue(nanovdb::Coord::Floor(data->iRay(data->iRay.t0() + tLight))) * lightRayDensity; //PerlinNoiseSampler::getInstance()->eval_density(samplePosLight);
				}

				float cos_theta = Utils::dotProduct(rayDirection, data->options.lightPosition);
				float light_attenuation = exp(-tau * light_step_size * sigma);

				result +=
					data->options.lightColor *							//light color
					light_attenuation *									// light ray transmission value
					sigma *												// scattering coefficient
					PhaseFunction::henyey_greenstein(data->options.heyneyGreensteinG, cos_theta) *	// phase function
					transmittance *										// ray current transmission value
					step_size;
			}
			//direct path to light
			//never happens?
			else {
				result +=
					data->options.lightColor *							//light color					
					sigma *												// scattering coefficient					
					transmittance *										// ray current transmission value
					step_size;
			}

			// the greater the value the more often we will break out from the marching loop
			int d = 2;
			if (transmittance < 1e-3) {
				// break
				if (data->randomGenerator->getFloat(0, 1) > 1.f / d)
					t = iRay.t1();
				// we continue but compensate
				else
					transmittance *= d;
			}
		}		
	}

	return Vec3f(result + transmittance * data->options.backgroundColor);
}

/// <summary>
/// data->rayOrigin is already in grid index-space here 
/// </summary>
/// <param name="data"></param>
/// <returns></returns>
bool IntegratorNanoVDBSimple::castLightRay(HandleIntersectionData* data) {
	using GridT = nanovdb::FloatGrid;
	using CoordT = nanovdb::Coord;
	using RealT = float;
	using Vec3T = nanovdb::Vec3<RealT>;
	using RayT = nanovdb::Ray<RealT>;

	auto rayDirection = Utils::normalize(data->rayDirection);

	// get an accessor.
	auto acc = data->sceneInfo->densityGrid->tree().getAccessor();

	Vec3T rayEye = { data->rayOrigin.x, data->rayOrigin.y, data->rayOrigin.z };
	Vec3T rayDir = { data->rayDirection.x, data->rayDirection.y, data->rayDirection.z };
	// generate ray.
	RayT wRay(rayEye, rayDir);
	// transform the ray to the grid's index-space.
	RayT iRay = wRay.worldToIndexF(*data->sceneInfo->densityGrid);
	// clip to bounds.
	if (iRay.clip(data->sceneInfo->gridBoundingBox) == false) {
		return false;
	}

	data->iRay = iRay;

	return true;
}