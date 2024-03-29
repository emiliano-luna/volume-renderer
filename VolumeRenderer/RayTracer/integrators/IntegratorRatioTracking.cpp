#include "IntegratorRatioTracking.h"
#include "../nanovdb/NanoVDB.h"
#include "../nanovdb/util/Ray.h"
#include "../nanovdb/util/IO.h"
#include "../nanovdb/util/Primitives.h"
#include "../nanovdb/util/GridStats.h"
#include "../nanovdb/fog_example/common.h"
#include "../Utils/PhaseFunction.h"
#include "../Utils/DirectionSampler.h"

Vec3f IntegratorRatioTracking::castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor)
{
	float densityMultiplier = data->options.lightRayDensityMultiplier;
	data->rayPDF = 1.0f;
	bool hasEmission = data->sceneInfo->temperatureGrid;

	data->depthRemaining = data->options.maxDepth;

	float tMin = data->options.stepSizeMin;
	float tMax = data->options.stepSizeMax;

	auto rayDirection = Utils::normalize(data->rayDirection);

	// get an accessor.
	auto acc = data->sceneInfo->densityGrid->tree().getAccessor();
	auto accEmission = acc;
	if (hasEmission)
		accEmission = data->sceneInfo->temperatureGrid->tree().getAccessor();

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
	//sigma_maj is very loose on fire.nvdb
	float sigma_maj = sigmaMax * (data->options.sigma_a + data->options.sigma_s);

	data->iRay = iRay;
	data->tFar = iRay.t0();	

	data->radiance = Vec3f(0.0f);
	data->transmission = 1.0f;

	bool terminated = false;

	while (!terminated && data->depthRemaining > 0) {
		// terminar temprano usando ruleta rusa?
		if (data->transmission < 0.05f) {
			float q = 0.75f;

			if (data->randomGenerator->getFloat(0.0f, 1.0f) < q)
				data->transmission = 0.0f;
			else
				data->transmission /= 1.0f - q;
		}

		if (data->transmission <= 0.0f) {
			terminated = true;
			break;
		}

		auto sigma = densityMultiplier * acc.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));
		auto mu_a = sigma * data->options.sigma_a;
		auto mu_s = sigma * data->options.sigma_s;
		auto mu_t = mu_a + mu_s;

		float pathLength = 0;
		if (sigma > 0.0f)
		{
			//sample free path length
			pathLength = -log(data->randomGenerator->getFloat(0.00001f, 1.0f)) / sigma_maj;
			pathLength *= data->options.stepSizeMultiplier;
			pathLength = Utils::clamp(tMin, tMax, pathLength);
		}
		else {
			pathLength = tMin * 10;
		}		

		data->tFar += pathLength;

		//if ray is outside medium return its weight
		if (data->tFar > data->iRay.t1()) {
			break;
		}

		if (sigma <= 0.0f)
			continue;
		
		float pAbsorption = mu_a / sigma_maj;
		float pScattering = mu_s / sigma_maj;
		float pNull = std::max<float>(0, 1 - pAbsorption - pScattering);

		//current sample transparency
		double sampleAttenuation = exp(-(pathLength) * mu_t);
		// attenuate volume object transparency by current sample transmission value
		data->transmission *= sampleAttenuation;
		data->rayPDF *= mu_t * sampleAttenuation;

		float sample = data->randomGenerator->getFloat(0.0f, 1.0f);

		if (hasEmission) {
			//add emission from medium scattering event
			float emission = accEmission.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));

			if (emission > 0.0f) {
				data->radiance +=
					data->transmission *
					mu_a *
					data->options.emissionColor *
					emission *
					pathLength;
			}
		}		

		//null-scattering
		if (sample < pNull) {}
		//absorption
		else if (sample < pNull + pAbsorption) {
			data->radiance += data->options.mediumColor * data->transmission;

			terminated = true;
		}
		//scattering
		else
		{			
			//if i run out of rebounds possible assume absorption
			if (data->depthRemaining-- < 0) {
				terminated = true;				
				break;
			}

			if (data->transmission > 0.0f) {
				//Sample direct lighting at volume-scattering event
				auto lightTransmission = directLightningRayMarch(data, 5.0f, sigma_maj);
				float cos_theta = Utils::dotProduct(data->rayDirection, data->options.lightPosition);
				float henyeyGreensteinPDF = PhaseFunction::henyey_greenstein(data->options.heyneyGreensteinG, cos_theta);

				data->radiance +=
					data->transmission *
					lightTransmission *
					data->options.lightColor *
					pathLength *
					henyeyGreensteinPDF;					// *
					
				//sigma / sigma_maj;//Ratio NEE

				auto scatteredDirection = data->directionSampler->sampleHenyeyGreenstein(
					data->options.heyneyGreensteinG,
					data->rayDirection,
					data->randomGenerator);

				//polar to cartesian coordinates
				nanovdb::Vec3<float> iRayOrigin = { data->iRay(data->tFar) };
				nanovdb::Vec3<float> rayDir = {
								scatteredDirection.x,
								scatteredDirection.y,
								scatteredDirection.z
				};

				if (data->options.useImportanceSampling)
				{
					float cos_theta = Utils::dotProduct(scatteredDirection, data->rayDirection);
					//multiply path pdf for direction sample pdf
					data->rayPDF *= PhaseFunction::henyey_greenstein(
						data->options.heyneyGreensteinG,
						cos_theta);
				}

				data->rayDirection = scatteredDirection;
				data->iRay = nanovdb::Ray<float>(iRayOrigin, rayDir);

				// clip to bounds.
				if (data->iRay.clip(data->sceneInfo->gridBoundingBox) == false) {
					std::cout << "scattering failed";

					terminated = true; 
					break;
				}

				data->tFar = data->iRay.t0();
			}
		}
	}

	if (terminated)
		return data->radiance;
	else {
		//Sample direct lighting when ray survives
		//auto lightTransmission = directLightningRayMarch(data, 5.0f, sigma_maj);
		//double cos_theta = Utils::dotProduct(data->rayDirection, data->options.lightPosition);
		//double henyeyGreensteinPDF = PhaseFunction::henyey_greenstein(data->options.heyneyGreensteinG, cos_theta);

		//data->radiance +=
		//	data->transmission *
		//	lightTransmission *
		//	data->options.lightColor *
		//	henyeyGreensteinPDF;

		return data->radiance + data->options.backgroundColor * data->transmission;
	}
		
}

float IntegratorRatioTracking::directLightningRayMarch(HandleIntersectionData* data, float maxStepSize, float sigma_maj) {
	auto transmission = 1.0f;
	float densityMultiplier = data->options.shadowRayDensityMultiplier;
	float tMin = data->options.stepSizeMin * 5;
	float tMax = data->options.stepSizeMax;

	auto acc = data->sceneInfo->densityGrid->tree().getAccessor();

	nanovdb::Vec3<float> lightDirection = { 
		data->options.lightPosition.x, 
		data->options.lightPosition.y, 
		data->options.lightPosition.z 
	};
	auto lightRay = nanovdb::Ray<float>(data->iRay(data->iRay.t0()), lightDirection);
	
	//clip to bounds.
	if (lightRay.clip(data->sceneInfo->gridBoundingBox) == false) {
		//ray is outside participating media so we assume it's reached the directional light
		return transmission;
	}

	auto tFar = lightRay.t0();

	while (true) {
		//Ajustar el tama�o del paso basado en la densidad
		float sigma = densityMultiplier * acc.getValue(nanovdb::Coord::Floor(lightRay(tFar)));
		auto mu_a = sigma * data->options.sigma_a;
		auto mu_s = sigma * data->options.sigma_s;
		auto mu_t = mu_a + mu_s;

		float stepSize = 0;
		if (sigma > 0.0f)
		{
			//sample free path length
			stepSize = -log(data->randomGenerator->getFloat(0.00001f, 1.0f)) / mu_t;
			stepSize *= data->options.stepSizeMultiplier;
			stepSize = Utils::clamp(tMin, tMax, stepSize);
		}
		else {
			stepSize = tMin * 100;
		}		 

		tFar += stepSize;

		//if ray is outside medium return its weight
		if (tFar > lightRay.t1()) {			
			return transmission;
		}

		if (sigma <= 0.0f)
			continue;

		//	# Calcular la transmisi�n del paso actual
		float transmissionStep = exp(-(stepSize) * mu_t);

		//	# Actualizar la transmisi�n total
		transmission *= transmissionStep;

		// terminar temprano usando ruleta rusa?
		if (transmission < 0.05f) {
			float q = 0.75f;

			if (data->randomGenerator->getFloat(0.0f, 1.0f) < q)
				transmission = 0.0f;
			else
				transmission /= 1.0f - q;
		}

		if (transmission <= 0.0f)
			return transmission;
	}
}