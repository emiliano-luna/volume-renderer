#include "BaseRenderer.h"
#include "..\Utils\MultithreadingHelper.h"
#include <ctime>
#include <algorithm>
#include <string>

void BaseRenderer::saveFile(Vec3f *framebuffer, int height, int width, const char* fileName) {
	FreeImage_Initialise();

	FIBITMAP* bitmap = FreeImage_Allocate(width, height, 24);
	RGBQUAD color;
	if (!bitmap)
		exit(1);

	for (uint32_t i = 0; i < height * width; ++i) {

		color.rgbRed = 255 * Utils::clamp(0, 1, framebuffer[i].x);
		color.rgbGreen = 255 * Utils::clamp(0, 1, framebuffer[i].y);
		color.rgbBlue = 255 * Utils::clamp(0, 1, framebuffer[i].z);
		FreeImage_SetPixelColor(bitmap, (i % width), height - (i / width) - 1, &color);	
	}

	if (!FreeImage_Save(FIF_PNG, bitmap, fileName, 0))
		std::cout << "Renderer - Ocurrió un error al guardar la imagen!!!" << std::endl;
	FreeImage_DeInitialise(); //Cleanup !
}

// generate primary ray direction
void BaseRenderer::renderRay(int i, int j, float pixelWidth, float pixelHeight, Vec3f* &pix, Vec3f* orig, float imageAspectRatio, float scale, HandleIntersectionData* data) {
	auto width = data->options.widthReference > 0 ? data->options.widthReference : data->options.width;
	auto height = data->options.heightReference > 0 ? data->options.heightReference : data->options.height;
	
	float x = (2 * (i + 0.5) / (float)width - 1) * imageAspectRatio * scale;
	float y = (1 - 2 * (j + 0.5) / (float)height) * scale;

	auto raysPerPixel = data->options.rayPerPixelCount;
	
	Vec3f dir; //= Utils::normalize(Vec3f(x, y, -1));
	if (raysPerPixel != 4 && raysPerPixel != 16)
		dir = Utils::normalize(Vec3f(x, y, -1));

	//El orden y, x, z es para matchear con el pitch roll y yaw del mï¿½todo (usa otro sistemas de coordenadas)
	Utils::rotate(data->options.cameraRotation.y, data->options.cameraRotation.x, data->options.cameraRotation.z, &dir);

	Vec3f color;
	
	for (size_t i = 0; i < raysPerPixel; i++)
	{
		if (raysPerPixel == 4)
		{
			if (i == 0)	dir = Utils::normalize(Vec3f(x + pixelWidth * 0.25, y + pixelHeight * 0.25, -1));
			if (i == 1)	dir = Utils::normalize(Vec3f(x + pixelWidth * 0.25, y - pixelHeight * 0.25, -1));
			if (i == 2)	dir = Utils::normalize(Vec3f(x - pixelWidth * 0.25, y + pixelHeight * 0.25, -1));
			if (i == 3)	dir = Utils::normalize(Vec3f(x - pixelWidth * 0.25, y - pixelHeight * 0.25, -1));
		}
		if (raysPerPixel == 16)
		{
			if (i == 0)	dir = Utils::normalize(Vec3f(x + pixelWidth * 0.25 + pixelWidth * 0.125, y + pixelHeight * 0.25 + pixelHeight * 0.125, -1));
			if (i == 1)	dir = Utils::normalize(Vec3f(x + pixelWidth * 0.25 - pixelWidth * 0.125, y + pixelHeight * 0.25 + pixelHeight * 0.125, -1));
			if (i == 2)	dir = Utils::normalize(Vec3f(x + pixelWidth * 0.25 + pixelWidth * 0.125, y + pixelHeight * 0.25 - pixelHeight * 0.125, -1));
			if (i == 3)	dir = Utils::normalize(Vec3f(x + pixelWidth * 0.25 - pixelWidth * 0.125, y + pixelHeight * 0.25 - pixelHeight * 0.125, -1));
			if (i == 4)	dir = Utils::normalize(Vec3f(x - pixelWidth * 0.25 + pixelWidth * 0.125, y + pixelHeight * 0.25 + pixelHeight * 0.125, -1));
			if (i == 5)	dir = Utils::normalize(Vec3f(x - pixelWidth * 0.25 - pixelWidth * 0.125, y + pixelHeight * 0.25 + pixelHeight * 0.125, -1));
			if (i == 6)	dir = Utils::normalize(Vec3f(x - pixelWidth * 0.25 + pixelWidth * 0.125, y + pixelHeight * 0.25 - pixelHeight * 0.125, -1));
			if (i == 7)	dir = Utils::normalize(Vec3f(x - pixelWidth * 0.25 - pixelWidth * 0.125, y + pixelHeight * 0.25 - pixelHeight * 0.125, -1));
			if (i == 8)	dir = Utils::normalize(Vec3f(x + pixelWidth * 0.25 + pixelWidth * 0.125, y - pixelHeight * 0.25 + pixelHeight * 0.125, -1));
			if (i == 9)	dir = Utils::normalize(Vec3f(x + pixelWidth * 0.25 - pixelWidth * 0.125, y - pixelHeight * 0.25 + pixelHeight * 0.125, -1));
			if (i == 10)	dir = Utils::normalize(Vec3f(x + pixelWidth * 0.25 + pixelWidth * 0.125, y - pixelHeight * 0.25 - pixelHeight * 0.125, -1));
			if (i == 11)	dir = Utils::normalize(Vec3f(x + pixelWidth * 0.25 - pixelWidth * 0.125, y - pixelHeight * 0.25 - pixelHeight * 0.125, -1));
			if (i == 12)	dir = Utils::normalize(Vec3f(x - pixelWidth * 0.25 + pixelWidth * 0.125, y - pixelHeight * 0.25 + pixelHeight * 0.125, -1));
			if (i == 13)	dir = Utils::normalize(Vec3f(x - pixelWidth * 0.25 - pixelWidth * 0.125, y - pixelHeight * 0.25 + pixelHeight * 0.125, -1));
			if (i == 14)	dir = Utils::normalize(Vec3f(x - pixelWidth * 0.25 + pixelWidth * 0.125, y - pixelHeight * 0.25 - pixelHeight * 0.125, -1));
			if (i == 15)	dir = Utils::normalize(Vec3f(x - pixelWidth * 0.25 - pixelWidth * 0.125, y - pixelHeight * 0.25 - pixelHeight * 0.125, -1));
		}

		data->rayOrigin = *orig;
		data->rayDirection = dir;
		data->objectId = -1;
		data->L_total_diffuse = Vec3f(0.0f);
		data->throughput = Vec3f(1.0f);
		data->randSeed = i * 10000 + j;

		//color += castRayNanoVDB(data, 0, 1);
		color += castRay(data, 0, 1);
	}

	*(pix++) = color / raysPerPixel;
}


void BaseRenderer::renderPixel(int i, int j, Options &options,
	SceneInfo* scene)
{
	Vec3f* framebuffer = new Vec3f[1];
	Vec3f* pix = framebuffer;

	HandleIntersectionData* data = new HandleIntersectionData();
		
	data->sceneInfo = scene;
	data->options = options;	
	data->throughput = 1;

	float scale = tan(Utils::deg2rad(options.fov * 0.5));
	float imageAspectRatio = options.width / (float)options.height;

	float x = (2 * (0.5) / (float)data->options.width - 1) * imageAspectRatio * scale;
	float y = (1 - 2 * (0.5) / (float)data->options.height) * scale;

	float xPlusOne = (2 * (1.5) / (float)data->options.width - 1) * imageAspectRatio * scale;
	float pixelWidth = xPlusOne - x;

	float yPlusOne = (1 - 2 * (1.5) / (float)data->options.height) * scale;
	float pixelHeight = yPlusOne - y;

	renderRay(i, j, pixelWidth, pixelHeight, pix, &options.cameraPosition, imageAspectRatio, scale, data);

	saveFile(framebuffer, 1, 1, "outPixel.png");

	std::cout << "Renderer - Imagen Guardada.";

	delete[] framebuffer;
}


void BaseRenderer::render(Options &options,
	SceneInfo* scene)
{
	Vec3f *framebuffer = new Vec3f[options.width * options.height];
	Vec3f *pix = framebuffer;

	unsigned concurrentThreadsSupported = std::thread::hardware_concurrency();

	std::chrono::steady_clock::time_point begin;
	begin = std::chrono::steady_clock::now();
	
	if (options.multiThreaded && concurrentThreadsSupported > 1)
	{
		int heightPerThread = options.multiThreadedChunkSize > 0 ? 
			options.multiThreadedChunkSize : 
			options.height / concurrentThreadsSupported;
		
		sceneData.pix = pix;
		sceneData.options = options;
		sceneData.heightPerThread = heightPerThread;
		sceneData.orig = &options.cameraPosition;

		auto multithreadingHelper = new MultithreadingHelper(heightPerThread, options.height / heightPerThread);

		HANDLE* myhandle = new HANDLE[concurrentThreadsSupported];

		for (uint32_t i = 0; i < concurrentThreadsSupported; ++i) {
			RenderThreadData* data = new RenderThreadData();

			//uint32_t* fromHeight = new uint32_t(heightPerThread * i);
			//uint32_t* toHeight = new uint32_t(heightPerThread * (i + 1));
			uint32_t* ipoint = new uint32_t(i);

			data->renderer = this;
			//data->fromHeight = fromHeight;
			//data->toHeight = toHeight;
			data->chunkHeight = new uint32_t(heightPerThread);
			data->i = ipoint;
			data->scene = scene;
			data->multiThreadingHelper = multithreadingHelper;

			myhandle[i] = (HANDLE)_beginthreadex(0, 0, &BaseRenderer::mythread, data, 0, 0);
			SetThreadAffinityMask(myhandle[i], static_cast<DWORD_PTR>(1) << i);
		}

		WaitForMultipleObjects(concurrentThreadsSupported, myhandle, true, INFINITE);

		for (int i = 0; i < concurrentThreadsSupported; ++i)
			CloseHandle(myhandle[i]);
	}
	else
	{		
		BaseRenderer::renderPartial(&options.cameraPosition, pix, options.heightStartOffset + 0, options.heightStartOffset + options.height, options, scene);
	}

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Renderer - Escena rendereada en: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;
	
	std::time_t end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char dateChars[26];
	ctime_s(dateChars, sizeof dateChars, &end_time);
	std::string dateString = std::string(dateChars);
	dateString = dateString.substr(4, 15);
	std::replace(dateString.begin(), dateString.end(), ':', '_');
	std::replace(dateString.begin(), dateString.end(), ' ', '_');

	bool isAreaImage = options.widthStartOffset > 0 || options.heightStartOffset > 0;
	
	std::stringstream fileNameStream;
	fileNameStream << dateString << "_";
	fileNameStream << options.renderer << "_";
	if (isAreaImage)
		fileNameStream << "area" << options.widthStartOffset << "_" << options.heightStartOffset << "_";
	else
		fileNameStream << "full" << "_";
	fileNameStream << "ray" << options.rayPerPixelCount << "_";
	fileNameStream << "boun" << (int)options.maxDepth;
	fileNameStream << ".png";

	saveFile(framebuffer, options.height, options.width, fileNameStream.str().c_str());

	std::cout << "Renderer - Imagen Guardada.";

	delete[] framebuffer;
}

unsigned int __stdcall BaseRenderer::mythread(void* data)
{
	RenderThreadData* threadData = static_cast<RenderThreadData*>(data);
	BaseRenderer* renderer = threadData->renderer;

	std::stringstream stream;
	stream << "Rendering thread " << *threadData->i << " - Starting" << std::endl;
	std::cout << stream.str();

	uint32_t chunkOffset;
	while (threadData->multiThreadingHelper->tryReservingChunk(chunkOffset)) {
		auto fromHeight = chunkOffset;
		auto toHeight = chunkOffset + *threadData->chunkHeight;

		//std::stringstream stream3;
		//stream3 << "	Rendering chunk " << *threadData->i << " from " << fromHeight << " to " << toHeight << std::endl;
		//std::cout << stream3.str();

		renderer->renderPartial(renderer->sceneData.orig, &renderer->sceneData.pix[renderer->sceneData.options.width * fromHeight],
			fromHeight, toHeight, renderer->sceneData.options, threadData->scene);
	}

	std::stringstream stream2;
	stream2 << "Rendering thread " << *threadData->i << " - Done" << std::endl;
	std::cout << stream2.str();

	return 0;
}

void BaseRenderer::renderPartial(Vec3f* orig, Vec3f* pix, uint32_t fromHeight, uint32_t toHeight, const Options &options, SceneInfo* scene) {
	float scale = tan(Utils::deg2rad(options.fov * 0.5));
	float imageAspectRatio = options.width / (float)options.height;

	HandleIntersectionData* data = new HandleIntersectionData();
		
	data->sceneInfo = scene;
	data->options = options;	
	data->throughput = 1;

	float x = (2 * (0.5) / (float)data->options.width - 1) * imageAspectRatio * scale;
	float y = (1 - 2 * (0.5) / (float)data->options.height) * scale;

	float xPlusOne = (2 * (1.5) / (float)data->options.width - 1) * imageAspectRatio * scale;
	float pixelWidth = xPlusOne - x;

	float yPlusOne = (1 - 2 * (1.5) / (float)data->options.height) * scale;
	float pixelHeight = yPlusOne - y;

	for (uint32_t j = fromHeight; j < toHeight; ++j) {
		for (uint32_t i = options.widthStartOffset; i < options.width + options.widthStartOffset; ++i) {
			renderRay(i, j, pixelWidth, pixelHeight, pix, orig, imageAspectRatio, scale, data);
		}
	}
}

