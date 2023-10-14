#include "RendererPBRTVolume.h"
#include "../nanovdb/NanoVDB.h"
#include "../nanovdb/util/Ray.h"
#include "../nanovdb/util/IO.h"
#include "../nanovdb/util/Primitives.h"
#include "../nanovdb/util/GridStats.h"
#include "../nanovdb/fog_example/common.h"
#include "../Utils/PhaseFunction.h"

Vec3f RendererPBRTVolume::castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor)
{
	Vec3f light_color{ 9.0f * 4.0f, 2.25f * 4.0f, 0 };
	Vec3f light_dir{ 0, 1, 0 };

	data->depthRemaining = data->options.maxDepth;
	data->rayWeight = 0.0f;

	float tMin = 0.01f;

	// heyney-greenstein asymmetry factor of the phase function
	float g = 0.0;

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

	data->iRay = iRay;
	data->tFar = iRay.t0();	

	data->radiance = Vec3f(0.0f);
	data->transmission = 1.0f;
	//MIS from VolPathIntegrator
	//weight for undirected radiance in MIS
	data->r_u = 1.0f;
	//weight for direct lighting in MIS
	data->r_l = 1.0f;

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

		//sample free path length
		float pathLength = tMin + -log(data->randomGenerator->getFloat(0, 1)) / sigmaMax;				
		data->tFar += pathLength;
						
		//if ray is outside medium
		if (data->tFar > data->iRay.t1()) { 
			break;
		}

		auto sigma = acc.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));		
		if (sigma <= 0.0f)
			continue;

		auto node = acc.getNodeInfo(nanovdb::Coord::Floor(data->iRay(data->tFar)));

		//get density at current position in the medium 
		//sigma maj is wildly loose sigmaMax aprox 1.5 vs 0.001 sigma or less at most points
		float sigma_maj = sigmaMax * (data->options.sigma_a + data->options.sigma_s); 
		
		//why does this work????	
		/*
		float sigma_maj =//((1 / sigma) / sigmaMax) *
			//(data->options.sigma_a + data->options.sigma_s);*/

		float pAbsorption = sigma * data->options.sigma_a / sigma_maj;
		float pScattering = sigma * data->options.sigma_s / sigma_maj;
		float pNull = std::max<float>(0, 1 - pAbsorption - pScattering);

		float sample = data->randomGenerator->getFloat(0, 1);

		//add emission from medium scattering event
		float emission = accEmission.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));

		if (emission > 0.0f) {
			//// Compute $\beta'$ at new path vertex
			//float pdf = sigma_maj * data->transmission;
			//auto betap = data->transmission * data->transmission / pdf;

			////compute rescaled path probability for absorption at path vertex
			//float r_e = data->r_u * sigma_maj * data->transmission / pdf;

			//// Update radiance for medium emission
			//if (r_e > 0.0f)
			//	data->radiance += betap * data->options.sigma_a * getEmission(data, emission / emissionMax) / r_e;
			data->radiance += data->transmission * pAbsorption * getEmission(data, emission / emissionMax);
		}		

		//null-scattering
		if (sample < pNull) {
		}
		//absorption
		else if (sample < pNull + pAbsorption) {
			//float emission = accEmission.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));

			//data->radiance += /*data->transmission **/ getEmission(data, emission / emissionMax);

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

			// Update _beta_ and _r_u_ for real-scattering event
			//float pdf = data->transmission * data->options.sigma_s;
			//data->transmission *= data->transmission * data->options.sigma_s / pdf;
			//data->r_u *= data->transmission * data->options.sigma_s / pdf;

			if (data->transmission > 0.0f && data->r_u > 0.0f) {
				//Sample direct lighting at volume-scattering event
				auto lightTransmission = directLightningRayMarch(data, 5.0f, sigmaMax);
				float cos_theta = Utils::dotProduct(data->rayDirection, light_dir);

				data->radiance += 
					lightTransmission * 
					light_color * 
					PhaseFunction::heyney_greenstein(g, cos_theta);

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
		return data->radiance + data->options.backgroundColor;
}

Vec3f RendererPBRTVolume::getEmission(HandleIntersectionData* data, float emissionWeight){
	Vec3f emissionColor{ 1, 0.5f, 0.1f };

	return emissionColor * emissionWeight;
}

float RendererPBRTVolume::directLightningRayMarch(HandleIntersectionData* data, float maxStepSize, float sigmaMax) {
	auto transmission = 1.0f;
	float tMin = 0.01f;
	auto acc = data->sceneInfo->densityGrid->tree().getAccessor();
	auto lightRay = nanovdb::Ray<float>(data->iRay);

	while (true) {
		//Ajustar el tamaño del paso basado en la densidad
		float sigma = acc.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));
		float stepSize = tMin + -log(data->randomGenerator->getFloat(0, 1)) / sigmaMax; //std::min(maxStepSize, maxStepSize * sigma / sigmaMax);

		//	# Calcular la transmisión del paso actual
		float transmissionStep = exp(-stepSize * sigma);

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