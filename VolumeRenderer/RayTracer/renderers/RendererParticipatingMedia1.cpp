#include "RendererParticipatingMedia1.h"
#include "../nanovdb/NanoVDB.h"
#include "../nanovdb/util/Ray.h"
#include "../nanovdb/util/IO.h"
#include "../nanovdb/util/Primitives.h"
#include "../nanovdb/util/GridStats.h"
#include "../nanovdb/fog_example/common.h"
#include "../Utils/PhaseFunction.h"

Vec3f RendererParticipatingMedia1::castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor)
{
	data->depthRemaining = data->options.maxDepth;
	data->rayWeight = 0.0f;

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

	auto* densityGrid = handle.grid<float>();
	if (!densityGrid)
		throw std::runtime_error("GridHandle does not contain a valid host grid");

	auto* emissionGrid = handleEmission.grid<float>();
	if (!emissionGrid)
		throw std::runtime_error("handleEmission does not contain a valid host grid");

	//get grid stats
	nanovdb::Extrema<float> ext = nanovdb::getExtrema(*densityGrid, densityGrid->indexBBox());
	nanovdb::Extrema<float> extEmission = nanovdb::getExtrema(*emissionGrid, emissionGrid->indexBBox());

	float              wBBoxDimZ = (float)densityGrid->worldBBox().dim()[2] * 2;
	Vec3T              wBBoxCenter = Vec3T(densityGrid->worldBBox().min() + densityGrid->worldBBox().dim() * 0.5f);
	nanovdb::CoordBBox treeIndexBbox = densityGrid->tree().bbox();

	data->bbox = &treeIndexBbox;

	RayGenOp<Vec3T> rayGenOp(wBBoxDimZ, wBBoxCenter);
	CompositeOp     compositeOp;

	// get an accessor.
	auto acc = densityGrid->tree().getAccessor();
	auto accEmission = emissionGrid->tree().getAccessor();

	Vec3T rayEye = { data->rayOrigin.x, data->rayOrigin.y, data->rayOrigin.z };
	Vec3T rayDir = { data->rayDirection.x, data->rayDirection.y, data->rayDirection.z };
	// generate ray.
	RayT wRay(rayEye, rayDir);
	// transform the ray to the grid's index-space.
	RayT iRay = wRay.worldToIndexF(*densityGrid);
	// clip to bounds.
	if (iRay.clip(treeIndexBbox) == false) {		
		return Vec3f(data->options.backgroundColor);
	}

	float density = 64.0f;
	float lightRayDensity = density * 0.5f;
	// integrate...
	const float step_size = 0.5f;
	//transparency
	//float       transmittance = 1.0f;
	//initialize volumetric color to 0
	Vec3f result = Vec3f(0.0f);
	
	//find sigmaMax, max density in the entire medium	
	float sigmaMax = ext.max();
	float emissionMax = ext.max();

	data->iRay = iRay;
	data->tFar = iRay.t0();	

	//ray starts at intersection with grid
	//auto rayPosition = iRay(iRay.t0());
	//data->rayOrigin = Vec3f(rayPosition[0], rayPosition[1], rayPosition[2]);
	//data->rayDirection = nanovdb::Vec3<float> direction = iRay.dir();

	while (data->depthRemaining > 0) {
		//sample free path length
		float pathLength = -log(Utils::getRandomFloat(0,1)) / sigmaMax;		
		//pathLength *= 0.1f;
				
		data->tFar += pathLength;
		//auto iNextPosition = data->iRay(data->tFar);

		//verify if ray intersecs geometry 
		//TODO

		//move ray sampled length in the participating medium
		//data->rayOrigin = data->rayOrigin + data->rayDirection * pathLength;
				
		//if ray is outside medium return its weight
		if (data->tFar > data->iRay.t1()) { 			
			//I'm outside the narrow band
			
			//TODO return ray weight
			//return;
			return Vec3f(data->options.backgroundColor);
		}

		//get density at current position in the medium
		float sigma = acc.getValue(CoordT::Floor(data->iRay(data->tFar)));
		sigma *= density;

		//do delta tracking
		if (Utils::getRandomFloat(0, 1) < sigma / sigmaMax) {
			//true collision
			data->depthRemaining--;

			float emission = accEmission.getValue(CoordT::Floor(data->iRay(data->tFar)));
			//float emission = accEmission.getValue(CoordT::Floor(data->nanoVDBRay(data->tFar)));

			//sample new direction
			handleIntersection(data, sigma * 4.0f / sigmaMax, emission / emissionMax, densityGrid);

			//TODO return ray weight
			//return;
		}
	}

	return Vec3f(data->rayWeight * emissionColor);
}

void RendererParticipatingMedia1::handleIntersection(HandleIntersectionData* data, float absorptionChance, float emissionChance, nanovdb::FloatGrid* densityGrid) {
	//photon interacts with medium, decide intersection type		
	float pEmission = emissionChance;
	float pAbsorption = (absorptionChance + pEmission > 1.0f) ? 1.0f - pEmission : absorptionChance;
	float pScattering = 1.0f - pEmission - pAbsorption;

	//we take a random chance
	auto random = Utils::getRandomFloat(0, 1);

	//absorption
	if (random < pAbsorption) {		
		data->rayWeight = 0;
		//end path
		data->depthRemaining = 0;
	}
	//scattering
	else if (random < pAbsorption + pScattering) {
		//use Henyey-Greenstein to get scattering direction

		//g parámetro de anisotropía (g=0 isotrópico; g>0 anisotropía hacia adelante; g<0 anisotropía hacia atrás)
		float g = 0.0f;

		float xi = Utils::getRandomFloat(-1, 1);

		//get random theta and phi
		//theta using Henyey-Greenstein function
		float aux = ((1 - g * g) / (1 + g - 2 * g * xi));
		float cos_theta = 1 / (2 * g) * (1 + g * g - (aux * aux));
		float theta = std::acos(cos_theta);
		//phi randomly with uniform distribution in [0, 2*pi0]
		float phi = Utils::getRandomFloat(0, 1) * 2 * M_PI;

		//polar to cartesian coordinates
		nanovdb::Vec3<float> iRayOrigin = { data->iRay(data->tFar)};
		nanovdb::Vec3<float> rayDir = { 1 * sin(theta) * cos(phi),
			1 * sin(theta) * sin(phi),
			1 * cos(theta) };
		// generate ray.
		data->iRay = nanovdb::Ray<float>(iRayOrigin, rayDir);
		// transform the ray to the grid's index-space.
		//data->iRay = wRay.worldToIndexF(*densityGrid);

		// clip to bounds.
		if (data->iRay.clip(*data->bbox) == false) {
			//return Vec3f(data->options.backgroundColor);
		}

		data->tFar = data->iRay.t0();
	}
	//emission
	else {
		data->rayWeight = emissionChance;
		//end path
		data->depthRemaining = 0;
	}
}

/// <summary>
/// data->rayOrigin is already in grid index-space here 
/// </summary>
/// <param name="data"></param>
/// <returns></returns>
bool RendererParticipatingMedia1::castLightRay(HandleIntersectionData* data) {
	//using GridT = nanovdb::FloatGrid;
	//using CoordT = nanovdb::Coord;
	//using RealT = float;
	//using Vec3T = nanovdb::Vec3<RealT>;
	//using RayT = nanovdb::Ray<RealT>;

	//auto rayDirection = Utils::normalize(data->rayDirection);

	//nanovdb::GridHandle<nanovdb::HostBuffer>& handle = data->sceneInfo->densityGrid;

	//auto* h_grid = handle.grid<float>();
	//if (!h_grid)
	//	throw std::runtime_error("GridHandle does not contain a valid host grid");

	//float              wBBoxDimZ = (float)h_grid->worldBBox().dim()[2] * 2;
	//Vec3T              wBBoxCenter = Vec3T(h_grid->worldBBox().min() + h_grid->worldBBox().dim() * 0.5f);
	//nanovdb::CoordBBox treeIndexBbox = h_grid->tree().bbox();

	//RayGenOp<Vec3T> rayGenOp(wBBoxDimZ, wBBoxCenter);
	//CompositeOp     compositeOp;

	//// get an accessor.
	//auto acc = h_grid->tree().getAccessor();

	//Vec3T rayEye = { data->rayOrigin.x, data->rayOrigin.y, data->rayOrigin.z };
	//Vec3T rayDir = { data->rayDirection.x, data->rayDirection.y, data->rayDirection.z };
	//// generate ray.
	//RayT wRay(rayEye, rayDir);
	//// transform the ray to the grid's index-space.
	//RayT iRay = wRay.worldToIndexF(*h_grid);
	//// clip to bounds.
	//if (iRay.clip(treeIndexBbox) == false) {
	//	return false;
	//}

	////data->tFar = iRay.t1();
	//data->nanoVDBRay = iRay;

	return true;
}