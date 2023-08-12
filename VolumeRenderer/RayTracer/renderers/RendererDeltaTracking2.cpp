#include "RendererDeltaTracking2.h"
#include "../nanovdb/NanoVDB.h"
#include "../nanovdb/util/Ray.h"
#include "../nanovdb/util/IO.h"
#include "../nanovdb/util/Primitives.h"
#include "../nanovdb/util/GridStats.h"
#include "../nanovdb/fog_example/common.h"
#include "../Utils/PhaseFunction.h"

Vec3f RendererDeltaTracking2::castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor)
{
	Vec3f light_color{ 1, 1, 1 };
	Vec3f light_dir{ 0, 1, 0 };

	float g = 0.0f;

	data->depthRemaining = data->options.maxDepth;

	auto rayDirection = Utils::normalize(data->rayDirection);

	auto densityAccesor = data->sceneInfo->densityGrid->tree().getAccessor();
	auto temperatureAccesor = data->sceneInfo->temperatureGrid->tree().getAccessor();

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

	Vec3f result = Vec3f(0.0f);
	
	//find sigmaMax, max density in the entire medium	
	float sigmaMax = data->sceneInfo->densityExtrema.max();
	float emissionMax = data->sceneInfo->temperatureExtrema.max();

	data->iRay = iRay;
	data->tFar = iRay.t0();	
	data->radiance = Vec3f(0.0f);
	data->transmission = 1.0f;

	while (data->depthRemaining > 0 && data->transmission > 0.01f) {
		//sample free path length
		float pathLength = -log(data->randomGenerator->getFloat(0, 1)) / sigmaMax;
				
		data->tFar += pathLength;

		//if ray is outside medium return its weight
		if (data->tFar > data->iRay.t1()) {
			break;
		}

		//get density at current position in the medium
		float sigma = densityAccesor.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));
		sigma *= 64.0f;

		//do delta tracking
		if (sigma > 0.0f && 
			data->randomGenerator->getFloat(0, 1) < sigma / sigmaMax) {
			//true collision
			data->depthRemaining--;

			float emission = temperatureAccesor.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));

			float cos_theta = Utils::dotProduct(data->rayDirection, light_dir);

			nanovdb::Vec3<float> lightRayOrigin = { data->iRay(data->tFar) };
			nanovdb::Vec3<float> lightRayDirection = { light_dir.x, light_dir.y, light_dir.z };
			auto lightRay = nanovdb::Ray<float>(lightRayOrigin, lightRayDirection);

			//add direct light contribution to radiance
			auto lightTransmission = directLightningRayMarch(data, lightRay, densityAccesor, 5.0f, sigmaMax);
			data->radiance +=
				lightTransmission *
				light_color *
				PhaseFunction::heyney_greenstein(g, cos_theta);

			//sample new direction
			handleIntersection(data, sigma / sigmaMax, emission / emissionMax, densityAccesor, sigmaMax);
		}
	}

	return Vec3f(data->radiance + data->options.backgroundColor * data->transmission);
}

void RendererDeltaTracking2::handleIntersection(HandleIntersectionData* data, float absorptionChance, float emissionChance, nanovdb::DefaultReadAccessor<float> gridAccesor, float sigmaMax) {
	Vec3f emissionColor{ 1, 0.5f, 0.1f };		
	
	//photon interacts with medium, decide intersection type		
	float pEmission = emissionChance;
	float pAbsorption = (absorptionChance + pEmission > 1.0f) ? 1.0f - pEmission : absorptionChance;
	float pScattering = 1.0f - pEmission - pAbsorption;

	//we take a random chance
	auto random = data->randomGenerator->getFloat(0, 1);

	//absorption
	if (random < pAbsorption) {		
		//auto transmission = directLightningRayMarch(data, gridAccesor, 5.0f, sigmaMax);

		//data->L_total_diffuse = transmission * light_color;
		//data->rayWeight = 0;
		//end path
		data->transmission = 0.0f;
	}
	//scattering
	else if (random < pAbsorption + pScattering) {
		//use Henyey-Greenstein to get scattering direction

		//g parámetro de anisotropía (g=0 isotrópico; g>0 anisotropía hacia adelante; g<0 anisotropía hacia atrás)
		float g = 0.0f;

		float theta;
		if (g != 0.0f) {
			float xi = data->randomGenerator->getFloat(-1, 1);

			//get random theta and phi
			//theta using Henyey-Greenstein function
			float aux = ((1 - g * g) / (1 + g - 2 * g * xi));
			float cos_theta = 1 / (2 * g) * (1 + g * g - (aux * aux));
			theta = std::acos(cos_theta);
		}
		else {
			theta = data->randomGenerator->getFloat(0, 1) * 2 * M_PI;
		}

		//phi randomly with uniform distribution in [0, 2*pi0]
		float phi = data->randomGenerator->getFloat(0, 1) * 2 * M_PI;

		//polar to cartesian coordinates
		nanovdb::Vec3<float> iRayOrigin = { data->iRay(data->tFar)};
		nanovdb::Vec3<float> rayDir = { 1 * sin(theta) * cos(phi),
			1 * sin(theta) * sin(phi),
			1 * cos(theta) };

		data->rayDirection = Vec3f(rayDir[0], rayDir[1], rayDir[2]);
		// generate ray.
		data->iRay = nanovdb::Ray<float>(iRayOrigin, rayDir);

		// clip to bounds.
		if (data->iRay.clip(data->sceneInfo->gridBoundingBox) == false) {}

		data->tFar = data->iRay.t0();
	}
	//emission
	else {
		data->radiance += emissionColor;
		//end path
		data->transmission = 0.0f;
	}
}

float RendererDeltaTracking2::directLightningRayMarch(HandleIntersectionData* data, nanovdb::Ray<float> lightRay, nanovdb::DefaultReadAccessor<float> gridAccesor, float maxStepSize, float sigmaMax) {
	auto transmission = 1.0f;

	while (true) {
		//Ajustar el tamaño del paso basado en la densidad
		float sigma = gridAccesor.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));
		float stepSize = std::min(maxStepSize, maxStepSize * sigma / sigmaMax);

		//	# Calcular la transmisión del paso actual
		float transmissionStep = exp(-stepSize * sigma);
		
		//	# Actualizar la transmisión total
		transmission *= transmissionStep;

		//	# Avanzar el rayo
		lightRay = nanovdb::Ray<float>(lightRay(lightRay.t0() + stepSize), lightRay.dir());

		// clip to bounds.
		if (lightRay.clip(data->sceneInfo->gridBoundingBox) == false) {
			//ray is outside participating media so we assume it's reached the directional light
			return transmission;
		}
	}
}

/// <summary>
/// data->rayOrigin is already in grid index-space here 
/// </summary>
/// <param name="data"></param>
/// <returns></returns>
bool RendererDeltaTracking2::castLightRay(HandleIntersectionData* data) {
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