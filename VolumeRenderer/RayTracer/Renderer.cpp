#include "Renderer.h"

SceneData Renderer::scene;

Renderer::Renderer()
{
}

void Renderer::saveFile(Vec3f *framebuffer, int height, int width, const char* fileName) {
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

// Compute refraction direction using Snell's law
//
// We need to handle with care the two possible situations:
//
//    - When the ray is inside the object
//
//    - When the ray is outside.
//
// If the ray is outside, you need to make cosi positive cosi = -N.I
//
// If the ray is inside, you need to invert the refractive indices and negate the normal N
Vec3f Renderer::refract(const Vec3f &I, const Vec3f &N, const float &ior)
{
	float cosi = Utils::clamp(-1, 1, Utils::dotProduct(I, N));
	float etai = 1, etat = ior;
	Vec3f n = N;
	if (cosi < 0) { cosi = -cosi; }
	else { std::swap(etai, etat); n = -N; }
	float eta = etai / etat;
	float k = 1 - eta * eta * (1 - cosi * cosi);
	return k < 0 ? 0.0f : eta * I + (eta * cosi - sqrtf(k)) * n;
}

//Compute Fresnel equation
//I is the incident view direction
//N is the normal at the intersection point
//ior is the material refractive index
//[out] kr is the amount of light reflected
void Renderer::fresnel(const Vec3f &I, const Vec3f &N, const float &ior, float &kr)
{
	float cosi = Utils::clamp(-1, 1, Utils::dotProduct(I, N));
	float etai = 1, etat = ior;
	if (cosi > 0) { std::swap(etai, etat); }
	// Compute sini using Snell's law
	float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
	// Total internal reflection
	if (sint >= 1) {
		kr = 1;
	}
	else {
		float cost = sqrtf(std::max(0.f, 1 - sint * sint));
		cosi = fabsf(cosi);
		float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
		float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
		kr = (Rs * Rs + Rp * Rp) / 2;
	}
	// As a consequence of the conservation of energy, transmittance is given by:
	// kt = 1 - kr;
}

Vec3f Renderer::castRay(
	BaseIntersectionHandler *intersectionHandler,
	HandleIntersectionData *data,
	uint32_t depth)
{
	if (depth > data->options.maxDepth) {
		return data->L_total_diffuse;
	}

	struct RTCRayQueryContext context;
	rtcInitRayQueryContext(&context);

	struct RTCRayHit rayhit;
	rayhit.ray.org_x = data->rayOrigin.x;rayhit.ray.org_y = data->rayOrigin.y;rayhit.ray.org_z = data->rayOrigin.z;
	rayhit.ray.dir_x = data->rayDirection.x;rayhit.ray.dir_y = data->rayDirection.y;rayhit.ray.dir_z = data->rayDirection.z;
	rayhit.ray.tnear = 0;rayhit.ray.tfar = std::numeric_limits<float>::infinity();
	rayhit.ray.mask = -1;rayhit.ray.flags = 0;
	rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

	//Intersects a single ray with the scene
	rtcIntersect1(data->sceneInfo->scene, &rayhit);

	if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
	{
		data->previousObjectId = data->objectId;
		data->previousHitPoint = data->hitPoint;
		
		data->objectId = data->sceneInfo->primitives[rayhit.hit.primID];
		data->hitPoint = rayhit.ray.tfar * data->rayDirection + data->rayOrigin;

		data->hitNormal = Vec3f(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z);

		data->tFar = rayhit.ray.tfar;		
		
		//if (intersectionHandler->HandleIntersection(data, depth)) {
		intersectionHandler->HandleIntersection(data, depth);

		//return data->L_total_diffuse;
		//}

		//depth++;

		//return castRay(intersectionHandler, data, depth);
	}

	//No diffuse hits
	else if (data->throughput == 1.0f) {
		data->L_total_diffuse = data->options.backgroundColor;
	}

	//scratchpixel1
	if (data->transmissionRemaining > 0)
		data->L_total_diffuse = data->L_total_diffuse * data->transmissionRemaining + data->throughput;

	return data->L_total_diffuse;
}

// generate primary ray direction
void Renderer::renderRay(int i, int j, Vec3f* &pix, Vec3f* orig, float imageAspectRatio, float scale, BaseIntersectionHandler* intersectionHandler, HandleIntersectionData* data) {
	float x = (2 * (i + 0.5) / (float)data->options.width - 1) * imageAspectRatio * scale;
	float y = (1 - 2 * (j + 0.5) / (float)data->options.height) * scale;
	
	Vec3f dir = Utils::normalize(Vec3f(x, y, -1));

	//El orden y, x, z es para matchear con el pitch roll y yaw del método (usa otro sistemas de coordenadas)
	Utils::rotate(data->options.cameraRotation.y, data->options.cameraRotation.x, data->options.cameraRotation.z, &dir);

	data->rayOrigin = *orig;
	data->rayDirection = dir;
	data->objectId = -1;
	data->transmissionRemaining = 0;
	data->L_total_diffuse = Vec3f(0.0f);
	data->throughput = Vec3f(1.0f);

	*(pix++) = castRay(intersectionHandler, data, 0);
}

void Renderer::renderPixel(int i, int j, Options &options,
	SceneInfo* scene)
{
	Vec3f* framebuffer = new Vec3f[1];
	Vec3f* pix = framebuffer;

	BaseIntersectionHandler* intersectionHandler = IntersectionHandlerFactory::GetIntersectionHandler(options.intersectionHandler);
	HandleIntersectionData* data = new HandleIntersectionData();
		
	data->sceneInfo = scene;
	data->options = options;	
	data->throughput = 1;

	float scale = tan(Utils::deg2rad(options.fov * 0.5));
	float imageAspectRatio = options.width / (float)options.height;

	Renderer::renderRay(i, j, pix, &options.cameraPosition, imageAspectRatio, scale, intersectionHandler, data);

	saveFile(framebuffer, 1, 1, "outPixel.png");

	std::cout << "Renderer - Imagen Guardada.";

	delete[] framebuffer;
}


void Renderer::render(Options &options,
	SceneInfo* scene)
{
	Vec3f *framebuffer = new Vec3f[options.width * options.height];
	Vec3f *pix = framebuffer;

	unsigned concurrentThreadsSupported = std::thread::hardware_concurrency();

	std::chrono::steady_clock::time_point begin;
	begin = std::chrono::steady_clock::now();
	
	//if (options.multiThreaded && concurrentThreadsSupported > 1)
	//{
	//	int heightPerThread = options.height / concurrentThreadsSupported;

	//	Renderer::scene.pix = pix;
	//	Renderer::scene.options = options;
	//	Renderer::scene.heightPerThread = heightPerThread;
	//	Renderer::scene.orig = &options.cameraPosition;

	//	HANDLE* myhandle = new HANDLE[concurrentThreadsSupported];

	//	for (uint32_t i = 0; i < concurrentThreadsSupported; ++i) {
	//		RenderThreadData* data = new RenderThreadData();

	//		uint32_t* fromHeight = new uint32_t(heightPerThread * i);
	//		uint32_t* toHeight = new uint32_t(heightPerThread * (i + 1));
	//		uint32_t* ipoint = new uint32_t(i);

	//		data->fromHeight = fromHeight;
	//		data->toHeight = toHeight;
	//		data->i = ipoint;
	//		data->scene = scene;

	//		myhandle[i] = (HANDLE)_beginthreadex(0, 0, &Renderer::mythread, data, 0, 0);
	//		SetThreadAffinityMask(myhandle[i], 1 << i);
	//	}

	//	WaitForMultipleObjects(concurrentThreadsSupported, myhandle, true, INFINITE);

	//	for (int i = 0; i < concurrentThreadsSupported; ++i)
	//		CloseHandle(myhandle[i]);
	//}
	//else
	//{		
		Renderer::renderPartial(&options.cameraPosition, pix, 0, options.height - 1, options, scene);
	//}

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Renderer - Escena rendereada en: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;
	
	saveFile(framebuffer, options.height, options.width, "out.png");

	std::cout << "Renderer - Imagen Guardada.";

	delete[] framebuffer;
}

unsigned int __stdcall Renderer::mythread(void* data)
{
	RenderThreadData* threadData = static_cast<RenderThreadData*>(data);

	Renderer::renderPartial(Renderer::scene.orig, &Renderer::scene.pix[Renderer::scene.options.width * Renderer::scene.heightPerThread * *threadData->i],
		*threadData->fromHeight, *threadData->toHeight, Renderer::scene.options, threadData->scene);

	return 0;
}

void Renderer::renderPartial(Vec3f* orig, Vec3f* pix, uint32_t fromHeight, uint32_t toHeight, const Options &options, SceneInfo* scene) {
	float scale = tan(Utils::deg2rad(options.fov * 0.5));
	float imageAspectRatio = options.width / (float)options.height;

	BaseIntersectionHandler* intersectionHandler = IntersectionHandlerFactory::GetIntersectionHandler(options.intersectionHandler);
	HandleIntersectionData* data = new HandleIntersectionData();
		
	data->sceneInfo = scene;
	data->options = options;	
	data->throughput = 1;

	for (uint32_t j = fromHeight; j < toHeight; ++j) {
		for (uint32_t i = 0; i < options.width; ++i) {
			Renderer::renderRay(i, j, pix, orig, imageAspectRatio, scale, intersectionHandler, data);
		}
	}
}

Renderer::~Renderer()
{
}
