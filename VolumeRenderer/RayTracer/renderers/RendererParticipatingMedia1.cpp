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

	// heyney-greenstein asymmetry factor of the phase function
	float g = 0.0;

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

	//find sigmaMax, max density in the entire medium	
	float sigmaMax = data->sceneInfo->densityExtrema.max();
	float emissionMax = data->sceneInfo->temperatureExtrema.max();

	float density = 64.0f;
	float lightRayDensity = density * 0.5f;
	// integrate...
	const float step_size = 0.5f;
	//transparency
	//float       transmittance = 1.0f;
	//initialize volumetric color to 0
	Vec3f result = Vec3f(0.0f);

	data->iRay = iRay;
	data->tFar = iRay.t0();	

	//ray starts at intersection with grid
	//auto rayPosition = iRay(iRay.t0());
	//data->rayOrigin = Vec3f(rayPosition[0], rayPosition[1], rayPosition[2]);
	//data->rayDirection = nanovdb::Vec3<float> direction = iRay.dir();

	while (data->depthRemaining > 0) {
		//sample free path length
		float pathLength = -log(data->randomGenerator->getFloat(0, 1)) / sigmaMax;
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
			//return Vec3f(data->options.backgroundColor);
			
			//?????????
			//return Vec3f(data->options.backgroundColor * data->throughput + data->L_total_diffuse);
			return Vec3f(data->options.backgroundColor);
		}

		//get density at current position in the medium
		float sigma = densityAccesor.getValue(CoordT::Floor(data->iRay(data->tFar)));
		sigma *= density;

		//do delta tracking
		if (data->randomGenerator->getFloat(0, 1) < sigma / sigmaMax) {
			//true collision
			data->depthRemaining--;

			float emission = temperatureAccesor.getValue(CoordT::Floor(data->iRay(data->tFar)));
			//float emission = accEmission.getValue(CoordT::Floor(data->nanoVDBRay(data->tFar)));

			//sample new direction
			handleIntersection(data, sigma * 16.0f / sigmaMax, 0.5f * emission / emissionMax, densityAccesor, sigmaMax);

			//data->L_total_diffuse += light_color * data->throughput;

			//TODO return ray weight
			//return;
		}
	}

	return Vec3f(data->L_total_diffuse);
}

void RendererParticipatingMedia1::handleIntersection(HandleIntersectionData* data, float absorptionChance, float emissionChance, nanovdb::DefaultReadAccessor<float> gridAccesor, float sigmaMax) {
	Vec3f light_dir{ 0, 1, 0 };
	Vec3f light_color{ 1, 1, 1 };
	Vec3f emissionColor{ 1, 0.5f, 0.1f };		
	
	//photon interacts with medium, decide intersection type		
	float pEmission = emissionChance;
	float pAbsorption = (absorptionChance + pEmission > 1.0f) ? 1.0f - pEmission : absorptionChance;
	float pScattering = 1.0f - pEmission - pAbsorption;

	//we take a random chance
	auto random = data->randomGenerator->getFloat(0, 1);

	//absorption
	if (random < pAbsorption) {		
		auto transmission = directLightningRayMarch(data, gridAccesor, 5.0f, sigmaMax);

		data->L_total_diffuse = transmission * light_color;
		//data->rayWeight = 0;
		//end path
		data->depthRemaining = 0;
	}
	//scattering
	else if (random < pAbsorption + pScattering) {
		//use Henyey-Greenstein to get scattering direction

		//g par�metro de anisotrop�a (g=0 isotr�pico; g>0 anisotrop�a hacia adelante; g<0 anisotrop�a hacia atr�s)
		float g = 0.0f;

		float xi = data->randomGenerator->getFloat(-1, 1);

		//get random theta and phi
		//theta using Henyey-Greenstein function
		float aux = ((1 - g * g) / (1 + g - 2 * g * xi));
		float cos_theta = 1 / (2 * g) * (1 + g * g - (aux * aux));
		float theta = std::acos(cos_theta);
		//phi randomly with uniform distribution in [0, 2*pi0]
		float phi = data->randomGenerator->getFloat(0, 1) * 2 * M_PI;

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
		if (data->iRay.clip(data->sceneInfo->gridBoundingBox) == false) {
			//return Vec3f(data->options.backgroundColor);
		}

		data->tFar = data->iRay.t0();
	}
	//emission
	else {
		//data->rayWeight = emissionChance;
		data->L_total_diffuse = emissionChance * emissionColor;
		//end path
		data->depthRemaining = 0;
	}
}

float RendererParticipatingMedia1::directLightningRayMarch(HandleIntersectionData* data, nanovdb::DefaultReadAccessor<float> gridAccesor, float maxStepSize, float sigmaMax) {
	//Verificar intersecciones con superficies al principio
		//interseccion = intersectar_superficie(rayo)
		//if interseccion is not None:
			//# El rayo choca con una superficie antes de llegar a la luz
			//return 0.0 # No hay contribuci�n de la luz
		//transmision_total = 1.0
	auto transmission = 1.0f;
	//TODO: make ray towards light, its made on participatingMediaTransmission
	auto ray = nanovdb::Ray<float>(data->iRay(data->iRay.t0()), data->iRay.dir());

	while (true) {
		//Ajustar el tama�o del paso basado en la densidad
		float sigma = gridAccesor.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));
		float stepSize = std::min(maxStepSize, maxStepSize * sigma / sigmaMax);

		//	# Calcular la transmisi�n del paso actual
		float transmissionStep = exp(-stepSize * sigma);
		
		//	# Actualizar la transmisi�n total
		transmission *= transmissionStep;

		//	# Avanzar el rayo
		ray = nanovdb::Ray<float>(ray(ray.t0() + stepSize), ray.dir());

		// clip to bounds.
		if (ray.clip(data->sceneInfo->gridBoundingBox) == false) {
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