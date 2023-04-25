#include "stdafx.h"
#include "XMLManager.h"

XMLManager::XMLManager()
{
}

Options* XMLManager::GetRendererOptions() {
	Options* options = new Options();

	pugi::xml_document doc;

	pugi::xml_parse_result result = doc.load_file("configRenderer.xml");

	//Options
	pugi::xml_node root = doc.child("options");

	options->radiusSearch = root.child("radius").text().as_float();
	options->fileName = root.child("fileName").text().as_string();

	options->width = root.child("width").text().as_int();
	options->height = root.child("height").text().as_int();
	options->multiThreaded = root.child("multiThreaded").text().as_bool();
	//options->antiAliasing = root.child("antiAliasing").text().as_bool();
	//options->colorDiffThreshold = root.child("antiAliasing").attribute("colorDiff").as_float();
	//options->auxImages = root.child("auxImages").text().as_bool();
	/*options->ambientLight = Vec3f(
		root.child("ambient").attribute("r").as_float(),
		root.child("ambient").attribute("g").as_float(),
		root.child("ambient").attribute("b").as_float()
	);*/

	options->fov = root.child("camera").attribute("fov").as_float();
	
	options->cameraPosition.x = root.child("camera").attribute("x").as_float();
	options->cameraPosition.y = root.child("camera").attribute("y").as_float();
	options->cameraPosition.z = root.child("camera").attribute("z").as_float();

	options->cameraRotation.x = root.child("camera").attribute("pitch").as_float();
	options->cameraRotation.y = root.child("camera").attribute("yaw").as_float();
	options->cameraRotation.z = root.child("camera").attribute("roll").as_float();

	return options;
}

PhotonMapOptions* XMLManager::GetPhotonMapOptions() {
	PhotonMapOptions* options = new PhotonMapOptions();

	pugi::xml_document doc;

	pugi::xml_parse_result result = doc.load_file("config.xml");

	//Options
	pugi::xml_node root = doc.child("options");

	options->photonCount = root.child("photons").text().as_int();
	options->modelName = root.child("model").text().as_string();
	options->fileName = root.child("fileName").text().as_string();

	options->lightPos.x = root.child("light").attribute("x").as_float();
	options->lightPos.y = root.child("light").attribute("y").as_float();
	options->lightPos.z = root.child("light").attribute("z").as_float();

	options->lightColor.x = root.child("light").attribute("r").as_float();
	options->lightColor.y = root.child("light").attribute("g").as_float();
	options->lightColor.z = root.child("light").attribute("b").as_float();
	
	return options;
}
