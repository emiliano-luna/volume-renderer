#define NOMINMAX

#pragma once
#ifndef VOLUMERENDERER_BASERENDERER
#define VOLUMERENDERER_BASERENDERER

#include <Windows.h>
#include "Process.h"
#include "../Utils\Types.h"
#include "../Utils\Utils.h"
#include "../IntersectionHandlers/BaseIntersectionHandler.h"
#include "FreeImage.h"
#include <vector>
#include <rtcore.h>
#include <stdio.h>
#include <math.h>
#include <chrono>
#include <thread>
#include "../Utils\MultithreadingHelper.h"

struct SceneData {
	Vec3f* pix;
	uint32_t heightPerThread;
	Options options;
	Vec3f* orig;
};

class BaseRenderer
{
private:
	SceneData sceneData;
public:		
	void saveFile(Vec3f * framebuffer, int height, int width, const char* fileName);	
	virtual Vec3f castRay(HandleIntersectionData* intersectionData, uint32_t depth, uint32_t reboundFactor) = 0;
	void renderRay(int i, int j, float pixelWidth, float pixelHeight, Vec3f * &pix, Vec3f * orig, float imageAspectRatio, float scale, HandleIntersectionData* data);
	void renderPartial(Vec3f* orig, Vec3f* pix, ThreadInfo* threadInfo, const Options &options, SceneInfo* scene);
	void renderPixel(int i, int j, Options &options, SceneInfo* scene);
	void render(Options &options, SceneInfo* scene);
	Vec3f assignPointToQuadrant(int i, int total);

	static unsigned int __stdcall mythread(void * data);
};

struct RenderThreadData {
	BaseRenderer* renderer;
	uint32_t* chunkHeight;
	//uint32_t* toHeight;
	uint32_t* i;
	SceneInfo* scene;
	MultithreadingHelper* multiThreadingHelper;
};
#endif