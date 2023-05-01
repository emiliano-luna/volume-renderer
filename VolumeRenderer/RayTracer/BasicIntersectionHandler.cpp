#include "BasicIntersectionHandler.h"

bool BasicIntersectionHandler::HandleIntersection(HandleIntersectionData *data, Vec3f &resultColor){	

	//Si estoy intersecando el mismo objeto, lo ignoro
	if (data->previousObjectId == data->objectId)
	{
		data->rayOrigin = data->hitPoint + data->rayDirection * 0.001;
		
		return false;
	}

	auto material = data->sceneInfo->materials[data->sceneInfo->shapes[data->objectId].mesh.material_ids[0]];

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

	resultColor = surfaceColor;

	return true;
}
