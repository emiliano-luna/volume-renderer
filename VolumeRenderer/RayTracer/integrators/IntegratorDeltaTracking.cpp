#include "IntegratorDeltaTracking.h"
#include "../nanovdb/NanoVDB.h"
#include "../nanovdb/util/Ray.h"
#include "../nanovdb/util/IO.h"
#include "../nanovdb/util/Primitives.h"
#include "../nanovdb/util/GridStats.h"
#include "../nanovdb/fog_example/common.h"
#include "../Utils/PhaseFunction.h"
#include "../Utils/DirectionSampler.h"

Vec3f IntegratorDeltaTracking::castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor)
{
	float pathPDF = 1.0f;
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
	
	Vec3f result = Vec3f(0.0f);

	//find sigmaMax, max density in the entire medium	
	float sigmaMax = data->sceneInfo->densityExtrema.max();
	float emissionMax = data->sceneInfo->temperatureExtrema.max();

	float sigma_maj = sigmaMax * (data->options.sigma_a + data->options.sigma_s);

	data->iRay = iRay;
	data->tFar = iRay.t0();	

	bool terminated = false;

	while (!terminated && data->depthRemaining > 0) {
		auto sigma = acc.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));
		auto mu_a = sigma * data->options.sigma_a;
		auto mu_s = sigma * data->options.sigma_s;
		auto mu_t = mu_a + mu_s;

		float pathLength = 0;
		//float distanceSamplePDF = 1.0f;
		if (sigma > 0.0f)		
		{
			//sample free path length
			auto sampledDistance = -log(data->randomGenerator->getFloat(0, 1)) / mu_t;
			pathLength = Utils::clamp(tMin, tMax, sampledDistance);
			//distanceSamplePDF = mu_t * exp(-pathLength * mu_t);
		}
		else {
			pathLength = tMax;
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

		float sample = data->randomGenerator->getFloat(0, 1);

		//null-scattering
		if (sample < pNull) 
		{
			if (data->options.useImportanceSampling)
				pathPDF *= pNull;
		}
		//absorption
		else if (sample < pNull + pAbsorption) {
			if (hasEmission) {
				float emission = accEmission.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));

				result +=
					data->options.emissionColor *
					mu_a;
			}

			result += data->options.mediumColor;

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

			//use Henyey-Greenstein to get scattering direction
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
				float cos_theta = Utils::dotProduct(rayDirection, data->rayDirection);
				//multiply path pdf for direction sample pdf
				pathPDF *= PhaseFunction::henyey_greenstein(
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

	//pathPDF *= 800000.0f;
	//we increment values, particularly those near 0, to avoid getting images that are too dark
	//https://www.wolframalpha.com/input?i=f%28x%29+%3D+-+%281+-+x%29+%5E+3+%2B+1
	/*if (pathPDF < 1.0f)
		pathPDF = -std::pow(1 - pathPDF, 5) + 1;*/

	//if (pathPDF > 1.0f)
	//	pathPDF = 1.0f;

	data->rayPDF = pathPDF;

	if (terminated)
	{
		return result;
	}
	else
		return (result + data->options.backgroundColor);
}