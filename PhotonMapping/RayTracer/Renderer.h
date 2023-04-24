#define NOMINMAX

#pragma once
#include <Windows.h>
#include "Process.h"
#include "Types.h"
#include "Utils.h"
#include "FreeImage.h"
#include <vector>
#include <embree3/rtcore.h>
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
	//static bool trace(const Vec3f & orig, const Vec3f & dir, const std::vector<Object*>& objects, int objectId, float & tNear, uint32_t & index, Object ** hitObject);
	static Vec3f castRay(const Vec3f & orig, const Vec3f & dir, SceneInfo* scene, int objectId, const Options & options, uint32_t depth, my_kd_tree_t* photons, std::vector<PhotonData>* photonData);
	//static void renderPixel(const int i, const int j, const Options & options/*, std::vector<Object*>& objects, std::vector<Light*>& lights*/);
	static void renderRay(int i, int j, Vec3f * &pix, Vec3f * orig, float imageAspectRatio, float scale, const Options & options, SceneInfo* scene, my_kd_tree_t* photons, std::vector<PhotonData>* photonData);
	static void renderPartial(Vec3f* orig, Vec3f* pix, uint32_t fromHeight, uint32_t toHeight, const Options &options, SceneInfo* scene, my_kd_tree_t* photons, std::vector<PhotonData>* photonData);
	
	static void renderPixel(int i, int j, Options &options, SceneInfo* scene, my_kd_tree_t* photons, std::vector<PhotonData>* photonData);
	static void render(Options &options, SceneInfo* scene, my_kd_tree_t* photons, std::vector<PhotonData>* photonData);

	static unsigned int __stdcall mythread(void * data);

	~Renderer();
};
