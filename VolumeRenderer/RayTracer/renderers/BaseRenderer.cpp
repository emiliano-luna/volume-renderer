#include "BaseRenderer.h"
#include "../nanovdb/NanoVDB.h"
#include "../nanovdb/util/Ray.h"
#include "../nanovdb/util/IO.h"
#include "../nanovdb/util/Primitives.h"
#include "../nanovdb/fog_example/common.h"

void BaseRenderer::saveFile(Vec3f *framebuffer, int height, int width, const char* fileName) {
	FreeImage_Initialise();

	FIBITMAP* bitmap = FreeImage_Allocate(width, height, 24);
	RGBQUAD color;
	if (!bitmap)
		exit(1);
	//Draws a g r a d i ent from b l u e to green :
	for (uint32_t i = 0; i < height * width; ++i) {

		color.rgbRed = 255 * Utils::clamp(0, 1, framebuffer[i].x);
		color.rgbGreen = 255 * Utils::clamp(0, 1, framebuffer[i].y);
		color.rgbBlue = 255 * Utils::clamp(0, 1, framebuffer[i].z);
		FreeImage_SetPixelColor(bitmap, (i % width), height - (i / width) - 1, &color);
		// Notice how we ’ re c a l l i n g the & ope rator on ” c o l o r ”
		// so t h a t we can pas s a p o int e r to the c o l o r s t r u c t .		
	}

	if (!FreeImage_Save(FIF_PNG, bitmap, fileName, 0))
		std::cout << "Renderer - Ocurrió un error al guardar la imagen!!!" << std::endl;
	FreeImage_DeInitialise(); //Cleanup !
}

// generate primary ray direction
void BaseRenderer::renderRay(int i, int j, Vec3f* &pix, Vec3f* orig, float imageAspectRatio, float scale, HandleIntersectionData* data) {
	float x = (2 * (i + 0.5) / (float)data->options.width - 1) * imageAspectRatio * scale;
	float y = (1 - 2 * (j + 0.5) / (float)data->options.height) * scale;
	
	Vec3f dir = Utils::normalize(Vec3f(x, y, -1));

	//El orden y, x, z es para matchear con el pitch roll y yaw del mï¿½todo (usa otro sistemas de coordenadas)
	Utils::rotate(data->options.cameraRotation.y, data->options.cameraRotation.x, data->options.cameraRotation.z, &dir);

	Vec3f color;
	for (size_t i = 0; i < data->options.rayPerPixelCount; i++)
	{
		data->rayOrigin = *orig;
		data->rayDirection = dir;
		data->objectId = -1;
		data->L_total_diffuse = Vec3f(0.0f);
		data->throughput = Vec3f(1.0f);

		//color += castRayNanoVDB(data, 0, 1);
		color += castRay(data, 0, 1);
	}

	*(pix++) = color / data->options.rayPerPixelCount;
}
//Vec3f BaseRenderer::castRayNanoVDB(
//	HandleIntersectionData* data,
//	uint32_t depth,
//	uint32_t reboundFactor)
//{
//	return runNanoVDB(data->sceneInfo->nanovdbGridHandle, data);
//}
//
//Vec3f BaseRenderer::runNanoVDB(nanovdb::GridHandle<nanovdb::HostBuffer>& handle, HandleIntersectionData* data)
//{
//	using GridT = nanovdb::FloatGrid;
//	using CoordT = nanovdb::Coord;
//	using RealT = float;
//	using Vec3T = nanovdb::Vec3<RealT>;
//	using RayT = nanovdb::Ray<RealT>;
//
//	auto width = data->options.width;
//	auto height = data->options.height;
//
//	auto* h_grid = handle.grid<float>();
//	if (!h_grid)
//		throw std::runtime_error("GridHandle does not contain a valid host grid");
//
//	float              wBBoxDimZ = (float)h_grid->worldBBox().dim()[2] * 2;
//	Vec3T              wBBoxCenter = Vec3T(h_grid->worldBBox().min() + h_grid->worldBBox().dim() * 0.5f);
//	nanovdb::CoordBBox treeIndexBbox = h_grid->tree().bbox();
//	/*std::cout << "Bounds: "
//		<< "[" << treeIndexBbox.min()[0] << "," << treeIndexBbox.min()[1] << "," << treeIndexBbox.min()[2] << "] -> ["
//		<< treeIndexBbox.max()[0] << "," << treeIndexBbox.max()[1] << "," << treeIndexBbox.max()[2] << "]" << std::endl;*/
//
//	RayGenOp<Vec3T> rayGenOp(wBBoxDimZ, wBBoxCenter);
//	CompositeOp     compositeOp;
//
//	// get an accessor.
//	auto acc = h_grid->tree().getAccessor();
//
//	Vec3T rayEye = { data->rayOrigin.x, data->rayOrigin.y, data->rayOrigin.z };
//	Vec3T rayDir = { data->rayDirection.x, data->rayDirection.y, data->rayDirection.z };
//	// generate ray.
//	RayT wRay(rayEye, rayDir);
//	// transform the ray to the grid's index-space.
//	RayT iRay = wRay.worldToIndexF(*h_grid);
//	// clip to bounds.
//	if (iRay.clip(treeIndexBbox) == false) {		
//		return Vec3f(0.0f);
//	}
//	// integrate...
//	const float dt = 0.5f;
//	float       transmittance = 1.0f;
//	for (float t = iRay.t0(); t < iRay.t1(); t += dt) {
//		float sigma = acc.getValue(CoordT::Floor(iRay(t))) * 0.1f;
//		transmittance *= 1.0f - sigma * dt;
//	}
//
//	return Vec3f(1.0f - transmittance);
//}

void BaseRenderer::renderPixel(int i, int j, Options &options,
	SceneInfo* scene)
{
	Vec3f* framebuffer = new Vec3f[1];
	Vec3f* pix = framebuffer;

	//BaseIntersectionHandler* intersectionHandler = IntersectionHandlerFactory::GetIntersectionHandler(options.intersectionHandler);
	HandleIntersectionData* data = new HandleIntersectionData();
		
	data->sceneInfo = scene;
	data->options = options;	
	data->throughput = 1;

	float scale = tan(Utils::deg2rad(options.fov * 0.5));
	float imageAspectRatio = options.width / (float)options.height;

	renderRay(i, j, pix, &options.cameraPosition, imageAspectRatio, scale, data);

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
		int heightPerThread = options.height / concurrentThreadsSupported;
		
		sceneData.pix = pix;
		sceneData.options = options;
		sceneData.heightPerThread = heightPerThread;
		sceneData.orig = &options.cameraPosition;

		HANDLE* myhandle = new HANDLE[concurrentThreadsSupported];

		for (uint32_t i = 0; i < concurrentThreadsSupported; ++i) {
			RenderThreadData* data = new RenderThreadData();

			uint32_t* fromHeight = new uint32_t(heightPerThread * i);
			uint32_t* toHeight = new uint32_t(heightPerThread * (i + 1));
			uint32_t* ipoint = new uint32_t(i);

			data->renderer = this;
			data->fromHeight = fromHeight;
			data->toHeight = toHeight;
			data->i = ipoint;
			data->scene = scene;

			myhandle[i] = (HANDLE)_beginthreadex(0, 0, &BaseRenderer::mythread, data, 0, 0);
			SetThreadAffinityMask(myhandle[i], 1 << i);
		}

		WaitForMultipleObjects(concurrentThreadsSupported, myhandle, true, INFINITE);

		for (int i = 0; i < concurrentThreadsSupported; ++i)
			CloseHandle(myhandle[i]);
	}
	else
	{		
		BaseRenderer::renderPartial(&options.cameraPosition, pix, 0, options.height - 1, options, scene);
	}

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Renderer - Escena rendereada en: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;
	
	saveFile(framebuffer, options.height, options.width, "out.png");

	std::cout << "Renderer - Imagen Guardada.";

	delete[] framebuffer;
}

unsigned int __stdcall BaseRenderer::mythread(void* data)
{
	RenderThreadData* threadData = static_cast<RenderThreadData*>(data);
	BaseRenderer* renderer = threadData->renderer;

	renderer->renderPartial(renderer->sceneData.orig, &renderer->sceneData.pix[renderer->sceneData.options.width * renderer->sceneData.heightPerThread * *threadData->i],
		*threadData->fromHeight, *threadData->toHeight, renderer->sceneData.options, threadData->scene);

	return 0;
}

void BaseRenderer::renderPartial(Vec3f* orig, Vec3f* pix, uint32_t fromHeight, uint32_t toHeight, const Options &options, SceneInfo* scene) {
	float scale = tan(Utils::deg2rad(options.fov * 0.5));
	float imageAspectRatio = options.width / (float)options.height;

	//BaseIntersectionHandler* intersectionHandler = IntersectionHandlerFactory::GetIntersectionHandler(options.intersectionHandler);
	HandleIntersectionData* data = new HandleIntersectionData();
		
	data->sceneInfo = scene;
	data->options = options;	
	data->throughput = 1;

	for (uint32_t j = fromHeight; j < toHeight; ++j) {
		for (uint32_t i = 0; i < options.width; ++i) {
			renderRay(i, j, pix, orig, imageAspectRatio, scale, data);
		}
	}
}
