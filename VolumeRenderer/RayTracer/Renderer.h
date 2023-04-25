#define NOMINMAX

#pragma once
#include <Windows.h>
#include "Process.h"
#include "Types.h"
#include "Utils.h"
#include "FreeImage.h"
#include <vector>
#include <rtcore.h>
#include <stdio.h>
#include <math.h>
#include "nanonflann\utils.h"
#include <chrono>
#include <thread>

struct RenderThreadData {
	uint32_t* fromHeight;
	uint32_t* toHeight;
	uint32_t* i;
	SceneInfo* scene;
	my_kd_tree_t* photons;
	std::vector<PhotonData>* photonData;
};

struct SceneData {
	Vec3f* pix;
	uint32_t heightPerThread;
	Options options;
	Vec3f* orig;
};

class Renderer
{
private:	
	Renderer();
public:	
	static SceneData scene;
	static void saveFile(Vec3f * framebuffer, int height, int width, const char* fileName);
	static Vec3f refract(const Vec3f & I, const Vec3f & N, const float & ior);
	static void fresnel(const Vec3f & I, const Vec3f & N, const float & ior, float & kr);
	static Vec3f castRay(const Vec3f & orig, const Vec3f & dir, SceneInfo* scene, int objectId, const Options & options, uint32_t depth);
	static void renderRay(int i, int j, Vec3f * &pix, Vec3f * orig, float imageAspectRatio, float scale, const Options & options, SceneInfo* scene);
	static void renderPartial(Vec3f* orig, Vec3f* pix, uint32_t fromHeight, uint32_t toHeight, const Options &options, SceneInfo* scene);
	
	static void renderPixel(int i, int j, Options &options, SceneInfo* scene);
	static void render(Options &options, SceneInfo* scene);

	static unsigned int __stdcall mythread(void * data);

	~Renderer();
};
