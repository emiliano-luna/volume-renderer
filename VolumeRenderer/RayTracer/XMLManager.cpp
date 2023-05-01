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

	auto models = root.child("models").children("model");

	for (auto model : models)
	{
		Model newModel;

		newModel.baseDir = model.attribute("baseDir").as_string();
		newModel.fileName = model.text().as_string();

		options->models.push_back(newModel);
	}

	options->intersectionHandler = root.child("intersectionHandler").text().as_string();

	options->maxDepth = root.child("maxDepth").text().as_int();

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
	options->backgroundColor = Vec3f(
		root.child("backgroundColor").attribute("r").as_float(),
		root.child("backgroundColor").attribute("g").as_float(),
		root.child("backgroundColor").attribute("b").as_float()
	);

	options->fov = root.child("camera").attribute("fov").as_float();
	
	options->cameraPosition.x = root.child("camera").attribute("x").as_float();
	options->cameraPosition.y = root.child("camera").attribute("y").as_float();
	options->cameraPosition.z = root.child("camera").attribute("z").as_float();

	options->cameraRotation.x = root.child("camera").attribute("pitch").as_float();
	options->cameraRotation.y = root.child("camera").attribute("yaw").as_float();
	options->cameraRotation.z = root.child("camera").attribute("roll").as_float();

	return options;
}
