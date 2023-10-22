#include "RendererPBRTSimple.h"
#include "../nanovdb/NanoVDB.h"
#include "../nanovdb/util/Ray.h"
#include "../nanovdb/util/IO.h"
#include "../nanovdb/util/Primitives.h"
#include "../nanovdb/util/GridStats.h"
#include "../nanovdb/fog_example/common.h"
#include "../Utils/PhaseFunction.h"

Vec3f RendererPBRTSimple::castRay(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor)
{
	data->depthRemaining = data->options.maxDepth;
	float tMin = 0.01f;

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

	float sigma_maj = sigmaMax * (data->options.sigma_a + data->options.sigma_s);

	data->iRay = iRay;
	data->tFar = iRay.t0();	

	bool terminated = false;

	while (!terminated && data->depthRemaining > 0) {
		//sample free path length
		float pathLength = tMin + -log(data->randomGenerator->getFloat(0, 1)) / sigmaMax;
				
		data->tFar += pathLength;
						
		//if ray is outside medium return its weight
		if (data->tFar > data->iRay.t1()) { 
			break;
		}

		auto sigma = acc.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));

		if (sigma <= 0.0f)
			continue;

		//get density at current position in the medium 
		/*float sigma_maj = 
			((1 / sigma) / sigmaMax) *
			(data->options.sigma_a + data->options.sigma_s);*/		

		float pAbsorption = sigma * data->options.sigma_a / sigma_maj;
		float pScattering = sigma * data->options.sigma_s / sigma_maj;
		float pNull = std::max<float>(0, 1 - pAbsorption - pScattering);

		float sample = data->randomGenerator->getFloat(0, 1);

		//null-scattering
		if (sample < pNull) {}
		//absorption
		else if (sample < pNull + pAbsorption) {
			float emission = accEmission.getValue(nanovdb::Coord::Floor(data->iRay(data->tFar)));

			result += 
				data->options.emissionColor * 
				emission / emissionMax;

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

	if (terminated)
		return result;
	else
		return result + data->options.backgroundColor;
}