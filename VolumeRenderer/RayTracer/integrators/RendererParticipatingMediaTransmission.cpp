#include "RendererParticipatingMediaTransmission.h"
#include "../nanovdb/NanoVDB.h"
#include "../nanovdb/util/Ray.h"
#include "../nanovdb/util/IO.h"
#include "../nanovdb/util/Primitives.h"
#include "../nanovdb/util/GridStats.h"
#include "../nanovdb/fog_example/common.h"
#include "../Utils/PhaseFunction.h"

Vec3f RendererParticipatingMediaTransmission::castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor)
{
	float tMin = 0.01f;

	data->depthRemaining = data->options.maxDepth;

	auto rayDirection = Utils::normalize(data->rayDirection);

		// get an accessor.
	auto acc = data->sceneInfo->densityGrid->tree().getAccessor();
	auto accEmission = data->sceneInfo->temperatureGrid->tree().getAccessor();

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

	float density = 64.0f;
	float lightRayDensity = density * 0.5f;
	//initialize volumetric color to 0
	Vec3f result = Vec3f(0.0f);
	
	float sigmaMax = data->sceneInfo->densityExtrema.max();
	float emissionMax = data->sceneInfo->temperatureExtrema.max();

	data->iRay = iRay;
	data->tFar = iRay.t0();	
	auto t1 = data->iRay.t1();

	data->radiance = Vec3f(0.0f);
	data->transmission = 1.0f;

	while (data->depthRemaining > 0) {
		//sample free path length
		float pathLength = tMin + -log(data->randomGenerator->getFloat(0, 1)) / sigmaMax;
				
		data->tFar += pathLength;

		t1 = data->iRay.t1();
				
		//if ray is outside medium return its weight
		if (data->tFar > data->iRay.t1()) { 			
			break;
		}

		//get density at current position in the medium
		float sigma = acc.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));
		sigma *= density;

		if (sigma <= 0.0f)
			continue;

		float stepAttenuation = exp(-pathLength * sigma / sigmaMax);
		data->transmission *= stepAttenuation;

		if (data->randomGenerator->getFloat(0, 1) < sigma / sigmaMax) {
			//true collision
			data->depthRemaining--;

			float emission = accEmission.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));

			float cos_theta = Utils::dotProduct(data->rayDirection, data->options.lightPosition);

			nanovdb::Vec3<float> lightRayOrigin = { data->iRay(data->tFar) };
			nanovdb::Vec3<float> lightRayDirection = { data->options.lightPosition.x, data->options.lightPosition.y, data->options.lightPosition.z };
			auto lightRay = nanovdb::Ray<float>(lightRayOrigin, lightRayDirection);

			//add direct light contribution to radiance
			auto lightTransmission = directLightningRayMarch(data, lightRay, acc, 5.0f, sigmaMax);
			data->radiance += 
				data->transmission * 
				lightTransmission *
				data->options.lightColor * 
				PhaseFunction::heyney_greenstein(data->options.heyneyGreensteinG, cos_theta);

			//sample new direction
			handleIntersection(data, sigma);

			//add emission
			data->radiance += data->transmission * data->options.emissionColor * emission;

			//if i run out of rebounds possible assume absorption
			if (data->depthRemaining <= 0)
				data->transmission = 0.0f;
		}

		// the greater the value the more often we will break out from the marching loop
		int d = 2;
		if (data->transmission < 1e-3) {
			// break
			if (data->randomGenerator->getFloat(0, 1) > 1.f / d)
				data->depthRemaining = 0;
			// we continue but compensate
			else
				data->transmission *= d;
		}
	}

	return Vec3f(data->radiance + data->transmission * data->options.backgroundColor);
}

void RendererParticipatingMediaTransmission::handleIntersection(HandleIntersectionData* data, float sigma) {
	//scattering
	float g = data->options.heyneyGreensteinG;

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
	if (data->iRay.clip(data->sceneInfo->gridBoundingBox) == false) {
		std::cout << "scattering failed";
	}

	data->tFar = data->iRay.t0();	
}

float RendererParticipatingMediaTransmission::directLightningRayMarch(HandleIntersectionData* data, nanovdb::Ray<float> lightRay, nanovdb::DefaultReadAccessor<float> gridAccesor, float maxStepSize, float sigmaMax) {
	float tMin = 0.01f;
	auto transmission = data->transmission;

	while (true) {
		//Ajustar el tamaño del paso basado en la densidad
		float sigma = gridAccesor.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));
		float stepSize = tMin + -log(data->randomGenerator->getFloat(0, 1)) / sigmaMax;
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