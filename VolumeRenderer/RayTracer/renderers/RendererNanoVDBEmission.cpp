#include "RendererNanoVDBEmission.h"
#include "../nanovdb/NanoVDB.h"
#include "../nanovdb/util/Ray.h"
#include "../nanovdb/util/IO.h"
#include "../nanovdb/util/Primitives.h"
#include "../nanovdb/fog_example/common.h"
#include <random>
#include "../Utils/PhaseFunction.h"

Vec3f RendererNanoVDBEmission::castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor)
{
	using GridT = nanovdb::FloatGrid;
	using CoordT = nanovdb::Coord;
	using RealT = float;
	using Vec3T = nanovdb::Vec3<RealT>;
	using RayT = nanovdb::Ray<RealT>;

	Vec3f light_dir{ 0, 1, 0 };
	Vec3f light_color{ 12, 12, 12 };
	Vec3f emissionColor{ 1, 0.5f, 0.1f };

	// heyney-greenstein asymmetry factor of the phase function
	float g = 0.0;

	auto rayDirection = Utils::normalize(data->rayDirection);

	nanovdb::GridHandle<nanovdb::HostBuffer>& handle = data->sceneInfo->densityGrid;
	nanovdb::GridHandle<nanovdb::HostBuffer>& handleEmission = data->sceneInfo->temperatureGrid;

	auto* h_grid = handle.grid<float>();
	if (!h_grid)
		throw std::runtime_error("GridHandle does not contain a valid host grid");

	auto* emission_grid = handleEmission.grid<float>();
	if (!emission_grid)
		throw std::runtime_error("handleEmission does not contain a valid host grid");

	float              wBBoxDimZ = (float)h_grid->worldBBox().dim()[2] * 2;
	Vec3T              wBBoxCenter = Vec3T(h_grid->worldBBox().min() + h_grid->worldBBox().dim() * 0.5f);
	nanovdb::CoordBBox treeIndexBbox = h_grid->tree().bbox();
	/*std::cout << "Bounds: "
		<< "[" << treeIndexBbox.min()[0] << "," << treeIndexBbox.min()[1] << "," << treeIndexBbox.min()[2] << "] -> ["
		<< treeIndexBbox.max()[0] << "," << treeIndexBbox.max()[1] << "," << treeIndexBbox.max()[2] << "]" << std::endl;*/

	RayGenOp<Vec3T> rayGenOp(wBBoxDimZ, wBBoxCenter);
	CompositeOp     compositeOp;

	// get an accessor.
	auto acc = h_grid->tree().getAccessor();
	auto accEmission = emission_grid->tree().getAccessor();

	Vec3T rayEye = { data->rayOrigin.x, data->rayOrigin.y, data->rayOrigin.z };
	Vec3T rayDir = { data->rayDirection.x, data->rayDirection.y, data->rayDirection.z };
	// generate ray.
	RayT wRay(rayEye, rayDir);
	// transform the ray to the grid's index-space.
	RayT iRay = wRay.worldToIndexF(*h_grid);
	// clip to bounds.
	if (iRay.clip(treeIndexBbox) == false) {		
		return Vec3f(data->options.backgroundColor);
	}

	float density = 64.0f;
	float lightRayDensity = density * 0.5f;
	// integrate...
	const float step_size = 0.5f;
	//transparency
	float       transmittance = 1.0f;
	//initialize volumetric color to 0
	Vec3f result = Vec3f(0.0f);

	for (float t = iRay.t0(); t < iRay.t1(); t += step_size) {
		//cast light ray

		float sigma = acc.getValue(CoordT::Floor(iRay(t))) * density;		
		auto emissionValue = accEmission.getValue(CoordT::Floor(iRay(t))) * 4.0f;
		Vec3f emission = emissionColor * emissionValue;

		//current sample transparency
		float sampleAttenuation = exp(-step_size * sigma);

		// attenuate volume object transparency by current sample transmission value
		transmittance *= sampleAttenuation;

		//prepare light ray
		auto rayWorldPosition = h_grid->indexToWorldF(iRay(t));		
		data->rayDirection = light_dir;
		data->rayOrigin = Vec3f(rayWorldPosition[0], rayWorldPosition[1], rayWorldPosition[2]);;

		if (sigma > 0 || emissionValue > 0){	
			//in shadow
			if (castLightRay(data)) {
				float tau = 0;

				float light_step_size = step_size * 20.0f;
				//auto distanceRayLightToExitInVolume = data->tFar;
				auto num_steps_light = std::ceil(data->iRay.t1() / light_step_size);

				for (size_t nl = 0; nl < num_steps_light; ++nl) {
					float tLight = light_step_size * (nl + 0.5);
					//Vec3f samplePosLight = samplePosition + tLight * light_dir;
					tau += acc.getValue(CoordT::Floor(data->iRay(data->iRay.t0() + tLight))) * lightRayDensity; //PerlinNoiseSampler::getInstance()->eval_density(samplePosLight);
				}

				float cos_theta = Utils::dotProduct(rayDirection, light_dir);
				float light_attenuation = exp(-tau * light_step_size * sigma);

				result +=
					light_color *										//light color
					light_attenuation *									// light ray transmission value
					//density *											// volume density at the sample position
					sigma *												// scattering coefficient
					PhaseFunction::heyney_greenstein(g, cos_theta) *	// phase function
					transmittance *										// ray current transmission value
					//emission *
					step_size;
			}
			//direct path to light
			else {
				result +=
					light_color *										//light color
					//light_attenuation *								// light ray transmission value
					//density *											// volume density at the sample position
					sigma *												// scattering coefficient
					//PhaseFunction::heyney_greenstein(g, cos_theta) *	// phase function
					transmittance *										// ray current transmission value
					//emission *
					step_size;
			}

			result += emission * transmittance * step_size;
		}		
	}

	return Vec3f(result + transmittance * data->options.backgroundColor);
}

/// <summary>
/// data->rayOrigin is already in grid index-space here 
/// </summary>
/// <param name="data"></param>
/// <returns></returns>
bool RendererNanoVDBEmission::castLightRay(HandleIntersectionData* data) {
	using GridT = nanovdb::FloatGrid;
	using CoordT = nanovdb::Coord;
	using RealT = float;
	using Vec3T = nanovdb::Vec3<RealT>;
	using RayT = nanovdb::Ray<RealT>;

	auto rayDirection = Utils::normalize(data->rayDirection);

	nanovdb::GridHandle<nanovdb::HostBuffer>& handle = data->sceneInfo->densityGrid;

	auto* h_grid = handle.grid<float>();
	if (!h_grid)
		throw std::runtime_error("GridHandle does not contain a valid host grid");

	float              wBBoxDimZ = (float)h_grid->worldBBox().dim()[2] * 2;
	Vec3T              wBBoxCenter = Vec3T(h_grid->worldBBox().min() + h_grid->worldBBox().dim() * 0.5f);
	nanovdb::CoordBBox treeIndexBbox = h_grid->tree().bbox();

	RayGenOp<Vec3T> rayGenOp(wBBoxDimZ, wBBoxCenter);
	CompositeOp     compositeOp;

	// get an accessor.
	auto acc = h_grid->tree().getAccessor();

	Vec3T rayEye = { data->rayOrigin.x, data->rayOrigin.y, data->rayOrigin.z };
	Vec3T rayDir = { data->rayDirection.x, data->rayDirection.y, data->rayDirection.z };
	// generate ray.
	RayT wRay(rayEye, rayDir);
	// transform the ray to the grid's index-space.
	RayT iRay = wRay.worldToIndexF(*h_grid);
	// clip to bounds.
	if (iRay.clip(treeIndexBbox) == false) {
		return false;
	}

	//data->tFar = iRay.t1();
	data->iRay = iRay;

	return true;
}