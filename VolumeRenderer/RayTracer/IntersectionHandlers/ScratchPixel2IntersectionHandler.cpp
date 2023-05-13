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
			auto distance = data->tFar;
			auto sigma_a = 1 - material.dissolve;
			//for now we use the diffuse color of the object as the scattering color
			auto scatteringColor = Vec3f(material.diffuse[0], material.diffuse[1], material.diffuse[2]);
			auto transmission = exp(-distance * sigma_a);

			//data->transmissionRemaining = transmission;
			data->throughput = scatteringColor * (1 - transmission);

			//data->rayOrigin = data->hitPoint + data->rayDirection * 0.001;

			return data->options.backgroundColor * transmission + scatteringColor * (1 - transmission);
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
