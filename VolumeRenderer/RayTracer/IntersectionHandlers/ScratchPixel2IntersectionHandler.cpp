#include "ScratchPixel2IntersectionHandler.h"

Vec3f ScratchPixel2IntersectionHandler::HandleIntersection(HandleIntersectionData *data, uint32_t depth, uint32_t reboundFactor)
{
	auto material = data->sceneInfo->materials[data->sceneInfo->shapes[data->objectId].mesh.material_ids[0]];

	//participating media	
	if (material.dissolve > 0)
	{
		//exit intersection
		if (data->previousObjectId == data->objectId)
		{
			float transparency = 1;
			Vec3f result = Vec3f(0.0f);
			auto distance = data->tFar;

			Vec3f light_dir{ 0, 1, 0 };
			Vec3f light_color{ 0.325, 0.075, 0.225 };
			auto sigma_a = 1 - material.dissolve;

			float step_size = 0.2;
			int ns = std::ceil(distance / step_size);
			step_size = distance / ns;

			auto rayDirection = Utils::normalize(data->rayDirection);
			auto rayOrigin = data->rayOrigin;

			for (size_t n = 0; n < ns; n++)
			{
				float t = step_size * (n + 0.5f);
				Vec3f samplePosition = rayOrigin + rayDirection * t;

				//current sample transparency
				float sampleAttenuation = exp(-step_size * sigma_a);

				// attenuate volume object transparency by current sample transmission value
				transparency *= sampleAttenuation;

				data->rayDirection = light_dir;
				data->rayOrigin = samplePosition + data->rayDirection * 0.001;				
				// In-Scattering. Find the distance traveled by light through 
				// the volume to our sample point. Then apply Beer's law.							
				if (Renderer::castSingleRay(data)){
					auto distanceRayLightToExitInVolume = data->tFar;

					float light_attenuation = exp(-distanceRayLightToExitInVolume * sigma_a);
					// attenuate in-scattering contrib. by the transmission of all samples accumulated so far
					result += transparency * light_color * light_attenuation * step_size;
				}		
			}						

			// combine background color and volumetric object color
			return data->options.backgroundColor * transparency + result;
		}
		else {
			data->rayOrigin = data->hitPoint + data->rayDirection * 0.001;

			return Renderer::castRay(this, data, depth, reboundFactor);
		}
	}

	//Si estoy intersecando el mismo objeto, lo ignoro
	if (data->previousObjectId == data->objectId)
	{
		data->rayOrigin = data->rayOrigin + data->rayDirection * 0.001;

		return Renderer::castRay(this, data, depth + 1, reboundFactor);
	}

	//Superficie Especular
	//Asumimos que no son emisivos
	if (material.diffuse[0] == 0 && material.diffuse[1] == 0 && material.diffuse[2] == 0)
	{
		auto normal = Utils::normalize(data->hitNormal);
		auto newDir = Utils::normalize(Utils::reflect(data->rayDirection, normal));

		data->rayDirection = newDir;
		data->rayOrigin = data->hitPoint + data->rayDirection * 0.001;

		return Renderer::castRay(this, data, depth + 1, reboundFactor);
	}

	//Superficie emisiva
	if (material.emission[0] > 0 || material.emission[1] > 0 || material.emission[2] > 0)
	{
		return data->throughput * Vec3f(material.emission);
	}

	//Superficie Difusa	
	//Calcular la irradiancia directa (E_directa)
	auto E_directa_i = DirectLightSampler::Sample(data->hitPoint, data->sceneInfo);
	//Calcular el término de reflectancia difusa (rho_i)
	auto rho_i = Vec3f(material.diffuse);
	//Actualizar el total de radiancia difusa
	data->L_total_diffuse += data->throughput * (E_directa_i * rho_i) * (1.0f / reboundFactor);
	//Actualizar el throughput para el siguiente rebote
	data->throughput *= rho_i;

	//TODO: deberia tomar el rebound count del último rebote difuso y no del rebote anterior siempre
	reboundFactor *= data->options.diffuseReboundCount[depth];

	for (size_t i = 0; i < data->options.diffuseReboundCount[depth]; i++)
	{
		//Muestrear la nueva dirección y actualizar el rayo para el siguiente rebote (usar distribución coseno)
		data->rayDirection = DirectionSampler::getCosineDistributionRebound(data->hitNormal);
		data->rayOrigin = data->hitPoint + data->rayDirection * 0.001;

		Renderer::castRay(this, data, depth + 1, reboundFactor);
	}

	return data->L_total_diffuse;
}
