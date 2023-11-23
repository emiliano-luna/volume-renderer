#include "IntegratorRatioTracking.h"
#include "../nanovdb/NanoVDB.h"
#include "../nanovdb/util/Ray.h"
#include "../nanovdb/util/IO.h"
#include "../nanovdb/util/Primitives.h"
#include "../nanovdb/util/GridStats.h"
#include "../nanovdb/fog_example/common.h"
#include "../Utils/PhaseFunction.h"

Vec3f IntegratorRatioTracking::castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor)
{
	data->depthRemaining = data->options.maxDepth;

	float tMin = 0.01f;
	float tMax = 1.0f;
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
	
	Vec3f result = Vec3f(0.0f);
	
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

		auto sigma = acc.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));
		auto mu_a = sigma * data->options.sigma_a;
		auto mu_s = sigma * data->options.sigma_s;
		auto mu_t = mu_a + mu_s;

		float pathLength = 0;
		if (sigma > 0.0f)
			//sample free path length
			pathLength -= log(data->randomGenerator->getFloat(0, 1)) / mu_t;

		pathLength = Utils::clamp(tMin, tMax, pathLength);

		data->tFar += pathLength;

		//if ray is outside medium return its weight
		if (data->tFar > data->iRay.t1()) {
			break;
		}

		if (sigma <= 0.0f)
			continue;
								
		auto node = acc.getNodeInfo(nanovdb::Coord::Floor(data->iRay(data->tFar)));
		
		float pAbsorption = mu_a / sigma_maj;
		float pScattering = mu_s / sigma_maj;
		float pNull = std::max<float>(0, 1 - pAbsorption - pScattering);

		//current sample transparency
		float sampleAttenuation = exp(-(pathLength - tMin) * mu_t);
		// attenuate volume object transparency by current sample transmission value
		data->transmission *= sampleAttenuation;

		float sample = data->randomGenerator->getFloat(0, 1);

		//add emission from medium scattering event
		float emission = accEmission.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));

		if (emission > 0.0f) {
			data->radiance +=
				data->transmission *
				pAbsorption *
				data->options.emissionColor *
				emission * 
				pathLength;
		}		

		//null-scattering
		if (sample < pNull) {}
		//absorption
		else if (sample < pNull + pAbsorption) {
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

				float g = data->options.heyneyGreensteinG;

				data->radiance +=
					data->transmission *
					lightTransmission *
					data->options.lightColor *
					PhaseFunction::heyney_greenstein(g, cos_theta);// *
					//sigma / sigma_maj;//Ratio NEE

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
				nanovdb::Vec3<float> iRayOrigin = { data->iRay(data->tFar) };
				nanovdb::Vec3<float> rayDir = { 1 * sin(theta) * cos(phi),
					1 * sin(theta) * sin(phi),
					1 * cos(theta) };

				data->rayDirection = Vec3f(rayDir[0], rayDir[1], rayDir[2]);
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
	else
		return data->radiance + data->options.backgroundColor * data->transmission;
}

float IntegratorRatioTracking::directLightningRayMarch(HandleIntersectionData* data, float maxStepSize, float sigma_maj) {
	auto transmission = 1.0f;
	float tMin = 0.01f;
	float tMax = 1.0f;

	auto acc = data->sceneInfo->densityGrid->tree().getAccessor();
	auto lightRay = nanovdb::Ray<float>(data->iRay);

	while (true) {
		//Ajustar el tamaño del paso basado en la densidad
		float sigma = acc.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));
		auto mu_a = sigma * data->options.sigma_a;
		auto mu_s = sigma * data->options.sigma_s;
		auto mu_t = mu_a + mu_s;

		float pathLength = 0;
		if (sigma > 0.0f)
			//sample free path length
			pathLength -= log(data->randomGenerator->getFloat(0, 1)) / mu_t;

		float stepSize = Utils::clamp(tMin, tMax, pathLength);

		//	# Calcular la transmisión del paso actual
		float transmissionStep = exp(-(stepSize - tMin) * mu_t);

		//	# Actualizar la transmisión total
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

		//	# Avanzar el rayo
		lightRay = nanovdb::Ray<float>(lightRay(lightRay.t0() + stepSize), lightRay.dir());

		// clip to bounds.
		if (lightRay.clip(data->sceneInfo->gridBoundingBox) == false) {
			//ray is outside participating media so we assume it's reached the directional light
			return transmission;
		}
	}
}