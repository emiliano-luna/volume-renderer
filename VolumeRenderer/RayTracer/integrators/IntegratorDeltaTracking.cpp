#include "IntegratorDeltaTracking.h"
#include "../nanovdb/NanoVDB.h"
#include "../nanovdb/util/Ray.h"
#include "../nanovdb/util/IO.h"
#include "../nanovdb/util/Primitives.h"
#include "../nanovdb/util/GridStats.h"
#include "../nanovdb/fog_example/common.h"
#include "../Utils/PhaseFunction.h"

Vec3f IntegratorDeltaTracking::castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor)
{
	float pathPDF = 1.0f;
	bool hasEmission = data->sceneInfo->temperatureGrid;

	data->depthRemaining = data->options.maxDepth;
	float tMin = 0.01f;
	float tMax = 0.5f;

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
	bool survived = false;

	while (!terminated && data->depthRemaining > 0) {
		auto sigma = acc.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));
		auto mu_a = sigma * data->options.sigma_a;
		auto mu_s = sigma * data->options.sigma_s;
		auto mu_t = mu_a + mu_s;

		float pathLength = 0;
		float distanceSamplePDF = 1.0f;
		if (sigma > 0.0f)		
		{
			//sample free path length
			auto sampledDistance = -log(data->randomGenerator->getFloat(0, 1)) / mu_t;
			pathLength = Utils::clamp(tMin, tMax, sampledDistance);
			distanceSamplePDF = mu_t * exp(-pathLength * mu_t);
		}
		else {
			pathLength = tMax;
		}

		data->tFar += pathLength;
										
		//if ray is outside medium return its weight
		if (data->tFar > data->iRay.t1()) { 
			survived = true;
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
			pathPDF *= pNull;
		}
		//absorption
		else if (sample < pNull + pAbsorption) {
			if (hasEmission) {
				float emission = accEmission.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));

				result +=
					data->options.emissionColor *
					emission / emissionMax;
			}

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
			//g parámetro de anisotropía (g=0 isotrópico; g>0 anisotropía hacia adelante; g<0 anisotropía hacia atrás)
			float g = data->options.heyneyGreensteinG;

			float cos_theta;
			float xi = data->randomGenerator->getFloat(0, 1);

			if (g != 0.0f) {		
				//get random theta and phi
				//theta using Henyey-Greenstein function
				float aux = ((1 - g * g) / (1 + g - 2 * g * xi));
				cos_theta = 1 / (2 * g) * (1 + g * g - (aux * aux));
			}
			else {
				cos_theta = 2.0 * xi - 1.0;
			}

			float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

			//phi randomly with uniform distribution in [0, 2*pi0]
			float phi = data->randomGenerator->getFloat(0, 1) * 2 * M_PI;
			//polar to cartesian coordinates
			nanovdb::Vec3<float> iRayOrigin = { data->iRay(data->tFar) };
			nanovdb::Vec3<float> rayDir = { 
				sin_theta * cos(phi),
				sin_theta * sin(phi),
				cos_theta };

			data->rayDirection = Vec3f(rayDir[0], rayDir[1], rayDir[2]);
			data->iRay = nanovdb::Ray<float>(iRayOrigin, rayDir);

			//multiply path pdf for distance sample pdf
			//we multiply first jump after scattering event only for now, not sure if this is ok
			//note that this isn't the actual jump distance 
			//but the one before clamped between tMin and tMax
			//pathPDF *= distanceSamplePDF;
			//multiply path pdf for direction sample pdf
			pathPDF *= PhaseFunction::henyey_greenstein(g, cos_theta);

			// clip to bounds.
			if (data->iRay.clip(data->sceneInfo->gridBoundingBox) == false) {
				std::cout << "scattering failed";
				terminated = true; 

				break;
			}

			data->tFar = data->iRay.t0();
		}
	}	

	pathPDF *= 800000.0f;

	if (pathPDF > 1.0f)
		pathPDF = 1.0f;

	if (terminated)
		return result * pathPDF;
	else
		return (result + data->options.backgroundColor) * pathPDF;
}