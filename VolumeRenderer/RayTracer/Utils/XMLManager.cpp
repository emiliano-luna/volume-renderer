#include "..\stdafx.h"
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

	Model densityFieldModel;

	auto densityFieldNode = root.child("densityField");

	densityFieldModel.baseDir = densityFieldNode.attribute("baseDir").as_string();
	densityFieldModel.fileName = densityFieldNode.text().as_string();

	options->densityField = densityFieldModel;

	options->sigma_s = root.child("sigma_s").text().as_float();
	options->sigma_a = root.child("sigma_a").text().as_float();

	auto rebounds = root.child("diffuseRebounds").children("rebound");

	for (auto rebound : rebounds)
	{
		options->diffuseReboundCount.push_back(rebound.text().as_int());
	}

	options->rayPerPixelCount = root.child("rayPerPixelCount").text().as_uint();
	options->integrator = root.child("integrator").text().as_string();

	options->maxDepth = root.child("maxDepth").text().as_int();

	options->radiusSearch = root.child("radius").text().as_float();
	options->fileName = root.child("fileName").text().as_string();

	options->width = root.child("width").text().as_int();
	options->widthStartOffset = root.child("width").attribute("startOffset").as_int();
	options->widthReference = root.child("width").attribute("reference").as_int();
	options->height = root.child("height").text().as_int();
	options->heightStartOffset = root.child("height").attribute("startOffset").as_int();
	options->heightReference = root.child("height").attribute("reference").as_int();
	options->multiThreaded = root.child("multiThreaded").text().as_bool();
	options->multiThreadedChunkSize = root.child("multiThreadedChunkSize").text().as_int();

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

	options->lightPosition.x = root.child("light").attribute("x").as_float();
	options->lightPosition.y = root.child("light").attribute("y").as_float();
	options->lightPosition.z = root.child("light").attribute("z").as_float();
	options->lightColor.x = root.child("light").attribute("r").as_float();
	options->lightColor.y = root.child("light").attribute("g").as_float();
	options->lightColor.z = root.child("light").attribute("b").as_float();

	options->emissionColor.x = root.child("emission").attribute("r").as_float();
	options->emissionColor.y = root.child("emission").attribute("g").as_float();
	options->emissionColor.z = root.child("emission").attribute("b").as_float();

	options->heyneyGreensteinG = root.child("heyneygreenstein_g").text().as_float();

	return options;
}
