#include "DiffuseIntersectionHandler.h"

/// <summary>
/// </summary>
/// <param name="data"></param>
/// <param name="L_total">Radiancia total en el punto de intersección, es la cantidad de energía lumínica emitida, 
/// reflejada o transmitida en una dirección específica por unidad de área aparente. Es la cantidad que buscamos en el path tracing</param>
/// <param name="reboundCountSoFar">Si en el primer rebote y en el segundo hay 5 rebotes difusos, al llegar a el tercer rebote este valor va a estar en 
/// 5*5 = 25, así sabemos que este rayo sólo aporta 1/25 de su radiancia al valor resultante </param>
/// <returns></returns>
Vec3f DiffuseIntersectionHandler::HandleIntersection(HandleIntersectionData* data, uint32_t depth, uint32_t reboundFactor)
{
	//Si estoy intersecando el mismo objeto, lo ignoro
	if (data->previousObjectId == data->objectId)
	{
		data->rayOrigin = data->rayOrigin + data->rayDirection * 0.001;

		return Renderer::castRay(this, data, depth + 1, reboundFactor);
	}

	auto material = data->sceneInfo->materials[data->sceneInfo->shapes[data->objectId].mesh.material_ids[0]];

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