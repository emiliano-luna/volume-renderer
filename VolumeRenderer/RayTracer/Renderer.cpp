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
	return k < 0 ? 0 : eta * I + (eta * cosi - sqrtf(k)) * n;
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

// Returns true if the ray intersects an object, false otherwise.
//
// \param orig is the ray origin
//
// \param dir is the ray direction
//
// \param objects is the list of objects the scene contains
//
//objectIndex, el índice del objeto que generó el rayo, en el vector de objetos
//
// \param[out] tNear contains the distance to the cloesest intersected object.
//
// \param[out] index stores the index of the intersect triangle if the interesected object is a mesh.
//
// \param[out] *hitObject stores the pointer to the intersected object (used to retrieve material information, etc.)
/*bool Renderer::trace(
	const Vec3f &orig, const Vec3f &dir,
	const std::vector<Object*> &objects, int objectId,
	float &tNear, uint32_t &index, Object **hitObject)
{
	*hitObject = nullptr;
	for (uint32_t k = 0; k < objects.size(); ++k) {
		float tNearK = kInfinity;
		uint32_t indexK;
		//tNearK > 0 para que no tome en cuenta las intersecciones con objetos posicionados por detrás de la dirección de la luz
		//objectIndex != k para que no detecte intersecciones con él mismo
		if (objects[k]->intersect(orig, dir, tNearK, indexK) && objects[k]->id != objectId && tNearK > 0 && tNearK < tNear) {
			*hitObject = objects[k];

			tNear = tNearK;
			index = indexK;			
		}
		float a = 1;
	}

	return (*hitObject != nullptr);
}*/

Vec3f Renderer::castRay(
	const Vec3f &orig, const Vec3f &dir,
	SceneInfo* scene,
	int objectId,
	const Options &options,
	uint32_t depth)
{
	Vec3f hitColor = options.backgroundColor;

	if (depth > options.maxDepth) {
		return hitColor;
	}

	/*
	* The intersect context can be used to set intersection
	* filters or flags, and it also contains the instance ID stack
	* used in multi-level instancing.
	*/
	struct RTCRayQueryContext context;
	rtcInitRayQueryContext(&context);

	/*
	 * The ray hit structure holds both the ray and the hit.
	 * The user must initialize it properly -- see API documentation
	 * for rtcIntersect1() for details.
	 */
	struct RTCRayHit rayhit;
	rayhit.ray.org_x = orig.x;
	rayhit.ray.org_y = orig.y;
	rayhit.ray.org_z = orig.z;
	rayhit.ray.dir_x = dir.x;
	rayhit.ray.dir_y = dir.y;
	rayhit.ray.dir_z = dir.z;
	rayhit.ray.tnear = 0;
	rayhit.ray.tfar = std::numeric_limits<float>::infinity();
	rayhit.ray.mask = -1;
	rayhit.ray.flags = 0;
	rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
	rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

	/*
	 * There are multiple variants of rtcIntersect. This one
	 * intersects a single ray with the scene.
	 */
	rtcIntersect1(scene->scene, &rayhit);

	//printf("%f, %f, %f: ", orig.x, orig.y, orig.z);
	if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
	{
		/* Note how geomID and primID identify the geometry we just hit.
		 * We could use them here to interpolate geometry information,
		 * compute shading, etc.
		 * Since there is only a single triangle in this scene, we will
		 * get geomID=0 / primID=0 for all hits.
		 * There is also instID, used for instancing. See
		 * the instancing tutorials for more information */

		auto hitPoint = rayhit.ray.tfar * dir + orig;
		auto shapeIndex = scene->primitives[rayhit.hit.primID];

		//Si estoy intersecando el mismo objeto, lo ignoro
		if (shapeIndex == objectId)
		{
			hitColor = castRay(hitPoint + dir * 0.001, dir, scene, shapeIndex, options, depth + 1);
		}
		else
		{		
			auto material = scene->materials[scene->shapes[shapeIndex].mesh.material_ids[0]];			

			//Superficie Especular
			if (material.diffuse[0] == 0 && material.diffuse[1] == 0 && material.diffuse[2] == 0)
			{
				auto normal = Utils::normalize(Vec3f(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z));

				auto newDir = Utils::normalize(Utils::reflect(dir, normal));

				hitColor = castRay(hitPoint + newDir * 0.001, newDir, scene, shapeIndex, options, depth + 1);
			}
			//Superficie Difusa
			else
			{
				//std::vector<std::pair<size_t, PointCoord>> matches;

				//const PointCoord query_pt[3] = { hitPoint.x, hitPoint.y, hitPoint.z };
				//nanoflann::SearchParams params;

				//auto matchesCount = photons->radiusSearch(&query_pt[0], options.radiusSearch, matches, params);

				auto surfaceColor = Vec3f(material.diffuse[0], material.diffuse[1], material.diffuse[2]);

				hitColor = surfaceColor;

				//for (std::pair<size_t, PointCoord> hit : matches) {
				//	hitColor += (surfaceColor * (1 / M_PI)) * (*photonData)[hit.first].color;
				//}

				//hitColor = hitColor * (float)(1 / (M_PI * (options.radiusSearch * options.radiusSearch)));
				//hitColor = hitColor * (float)(1 / (M_PI * 50));
				//hitColor = Vec3f(1 / (float)(rayhit.hit.primID + 1));

				/*auto maxColor = std::max({ hitColor.x, hitColor.y, hitColor.z });
				if (maxColor > 1)
				{
					hitColor = hitColor * (1 / maxColor);
				}*/
			}
		}	
	}

	return hitColor;
}

// generate primary ray direction
void Renderer::renderRay(int i, int j, Vec3f* &pix, Vec3f* orig, float imageAspectRatio, float scale, const Options &options, 
												SceneInfo* scene) {
	float x = (2 * (i + 0.5) / (float)options.width - 1) * imageAspectRatio * scale;
	float y = (1 - 2 * (j + 0.5) / (float)options.height) * scale;
	
	Vec3f dir = Utils::normalize(Vec3f(x, y, -1));

	//El orden y, x, z es para matchear con el pitch roll y yaw del método (usa otro sistemas de coordenadas)
	Utils::rotate(options.cameraRotation.y, options.cameraRotation.x, options.cameraRotation.z, &dir);

	*(pix++) = castRay(*orig, dir, scene, -1, options, 0);
}

void Renderer::renderPixel(int i, int j, Options &options,
	SceneInfo* scene)
{
	Vec3f *framebuffer = new Vec3f[1];
	Vec3f *pix = framebuffer;

	float scale = tan(Utils::deg2rad(options.fov * 0.5));
	float imageAspectRatio = options.width / (float)options.height;
	Vec3f orig(0, 1, 2.25);

	Renderer::renderRay(i, j, pix, &orig, imageAspectRatio, scale, options, scene);

	std::cout << "Escena rendereada - tiempo transcurrido: " << std::endl;
	   
	saveFile(framebuffer, 1, 1, ("outPixel_" + std::to_string(i) + "_" + std::to_string(j) + ".png").c_str());

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

	for (uint32_t j = fromHeight; j < toHeight; ++j) {
		for (uint32_t i = 0; i < options.width; ++i) {
			Renderer::renderRay(i, j, pix, orig, imageAspectRatio, scale, options, scene);
		}
	}
}

Renderer::~Renderer()
{
}
