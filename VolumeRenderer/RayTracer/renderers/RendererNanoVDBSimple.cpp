#include "RendererNanoVDBSimple.h"
#include "../nanovdb/NanoVDB.h"
#include "../nanovdb/util/Ray.h"
#include "../nanovdb/util/IO.h"
#include "../nanovdb/util/Primitives.h"
#include "../nanovdb/fog_example/common.h"
#include <random>

Vec3f RendererNanoVDBSimple::castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor)
{
	using GridT = nanovdb::FloatGrid;
	using CoordT = nanovdb::Coord;
	using RealT = float;
	using Vec3T = nanovdb::Vec3<RealT>;
	using RayT = nanovdb::Ray<RealT>;

	auto width = data->options.width;
	auto height = data->options.height;

	nanovdb::GridHandle<nanovdb::HostBuffer>& handle = data->sceneInfo->nanovdbGridHandle;

	auto* h_grid = handle.grid<float>();
	if (!h_grid)
		throw std::runtime_error("GridHandle does not contain a valid host grid");

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
	// integrate...
	const float step_size = 0.5f;
	//transparency
	float       transmittance = 1.0f;
	//initialize volumetric color to 0
	Vec3f result = Vec3f(0.0f);

	for (float t = iRay.t0(); t < iRay.t1(); t += step_size) {
		//cast light ray

		float sigma = acc.getValue(CoordT::Floor(iRay(t))) * 0.1f;

		//current sample transparency
		float sampleAttenuation = exp(-step_size * sigma);

		// attenuate volume object transparency by current sample transmission value
		transmittance *= sampleAttenuation;

		if (sigma > 0) {
			result +=
				//light_color *										//light color
				//light_attenuation *									// light ray transmission value
				//density *											// volume density at the sample position
				sigma *											// scattering coefficient
				//PhaseFunction::heyney_greenstein(g, cos_theta) *	// phase function
				transmittance *										// ray current transmission value
				step_size;
		}		
	}

	return Vec3f(result + transmittance * data->options.backgroundColor);
}