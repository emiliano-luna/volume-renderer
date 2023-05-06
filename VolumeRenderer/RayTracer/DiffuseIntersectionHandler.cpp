#include "DiffuseIntersectionHandler.h"

/// <summary>
/// </summary>
/// <param name="data"></param>
/// <param name="L_total">Radiancia total en el punto de intersección, es la cantidad de energía lumínica emitida, 
/// reflejada o transmitida en una dirección específica por unidad de área aparente. Es la cantidad que buscamos en el path tracing</param>
/// <returns></returns>
bool DiffuseIntersectionHandler::HandleIntersection(HandleIntersectionData* data, Vec3f& L_total) 
{
	L_total = Vec3f(0.0f);

	//Si estoy intersecando el mismo objeto, lo ignoro
	if (data->previousObjectId == data->objectId)
	{
		data->rayOrigin = data->hitPoint + data->rayDirection * 0.001;

		return false;
	}

	auto material = data->sceneInfo->materials[data->sceneInfo->shapes[data->objectId].mesh.material_ids[0]];

	//Superficie Especular
	//Asumimos que no son emisivos
	if (material.diffuse[0] == 0 && material.diffuse[1] == 0 && material.diffuse[2] == 0)
	{
		auto normal = Utils::normalize(data->hitNormal);
		auto newDir = Utils::normalize(Utils::reflect(data->rayDirection, normal));

		data->rayOrigin = data->hitPoint + data->rayDirection * 0.001;
		data->rayDirection = newDir;

		return false;
	}

	//Superficie emisiva
	if (material.emission[0] > 0 || material.emission[1] > 0 || material.emission[2] > 0) 
	{
		L_total += data->throughput * Vec3f(material.emission);

		return true;
	}

	//Superficie Difusa
	L_total = Vec3f(material.diffuse[0], material.diffuse[1], material.diffuse[2]);

	return true;
}