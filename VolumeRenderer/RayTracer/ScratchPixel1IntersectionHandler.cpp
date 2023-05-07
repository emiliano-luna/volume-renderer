#include "ScratchPixel1IntersectionHandler.h"

bool ScratchPixel1IntersectionHandler::HandleIntersection(HandleIntersectionData *data, uint32_t depth){

	//Si estoy intersecando el mismo objeto, lo ignoro
	//if (data->previousObjectId == data->objectId)
	//{
	//	data->rayOrigin = data->hitPoint + data->rayDirection * 0.001;
	//	
	//	return false;
	//}
	
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

			data->transmissionRemaining = transmission;
			data->throughput = scatteringColor * (1 - transmission);

			data->rayOrigin = data->hitPoint + data->rayDirection * 0.001;
		}

		return false;
	}

	//Superficie Especular
	if (material.diffuse[0] == 0 && material.diffuse[1] == 0 && material.diffuse[2] == 0)
	{
		auto normal = Utils::normalize(data->hitNormal);
		auto newDir = Utils::normalize(Utils::reflect(data->rayDirection, normal));
						
		data->rayOrigin = data->hitPoint + data->rayDirection * 0.001;
		data->rayDirection = newDir;

		return false;
	}

	//Superficie Difusa
	auto surfaceColor = Vec3f(material.diffuse[0], material.diffuse[1], material.diffuse[2]);

	if (data->transmissionRemaining > 0)
		data->L_total_diffuse = surfaceColor * data->transmissionRemaining + data->throughput;
	else
		data->L_total_diffuse = surfaceColor;

	return true;
}
