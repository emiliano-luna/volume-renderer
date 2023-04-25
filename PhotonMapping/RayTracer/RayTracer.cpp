// MathLibrary.cpp : Defines the exported functions for the DLL.
#include "stdafx.h" // use stdafx.h in Visual Studio 2017 and earlier
//#include <utility>
//#include <limits.h>
#include "RayTracer.h"
#include "minimal.h"
#include "SceneRenderer.h"
#include "PhotonMapper.h"
#include "FileManager.h"
#include "Types.h"
#include <chrono>
#include "minimal.h"

void ray_tracer_test()
{
	MinimalTutorial::main2();
}

void GeneratePhotonMap()
{
	printf("PhotonMap - Inciando.\n");
	   
	auto options = FileManager::GetPhotonMapOptions();

	std::chrono::steady_clock::time_point begin;
	begin = std::chrono::steady_clock::now();

	auto mapper = new PhotonMapper();

	auto photonMap = mapper->Generate(options);

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "PhotonMap - Mapa de fotones creado en: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;

	begin = std::chrono::steady_clock::now();

	FileManager::SavePhotonMap(photonMap, options);

	end = std::chrono::steady_clock::now();
	std::cout << "PhotonMap - Archivo guardado en: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;

	printf("PhotonMap - Generado.");
}

void RenderScene()
{
	auto renderer = new SceneRenderer();

	auto configOptions = FileManager::GetRendererOptions();

	//std::cout << "Ingrese el nombre del mapa de fotones" << std::endl;
	//std::string photonMapName;
	//std::cin >> photonMapName;

	std::string modelName;

	std::chrono::steady_clock::time_point begin;
	begin = std::chrono::steady_clock::now();

	auto photonMap = FileManager::ReadPhotonMap(configOptions->fileName, &modelName);//PhotonMapper::Generate();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Renderer - Mapa de fotones cargado en: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;

	configOptions->maxDepth = 5;	

	//MinimalTutorial::main2();
	renderer->RenderScene(photonMap, modelName, *configOptions);

	std::getchar();
}
