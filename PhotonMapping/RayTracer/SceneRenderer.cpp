/* ======================================================================== //
// Copyright 2009-2019 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== */
#pragma once
#include "stdafx.h"
#include "SceneRenderer.h"

/*
 * A minimal tutorial.
 *
 * It demonstrates how to intersect a ray with a single triangle. It is
 * meant to get you started as quickly as possible, and does not output
 * an image.
 *
 * For more complex examples, see the other tutorials.
 *
 * Compile this file using
 *
 *   gcc -std=c99 \
 *       -I<PATH>/<TO>/<EMBREE>/include \
 *       -o minimal \
 *       minimal.c \
 *       -L<PATH>/<TO>/<EMBREE>/lib \
 *       -lembree3
 *
 * You should be able to compile this using a C or C++ compiler.
 */

 /*
  * This is only required to make the tutorial compile even when
  * a custom namespace is set.
  */
#if defined(RTC_NAMESPACE_OPEN)
RTC_NAMESPACE_OPEN
#endif

/*
 * Cast a single ray with origin (ox, oy, oz) and direction
 * (dx, dy, dz).
 */
void SceneRenderer::castRay(RTCScene scene,
	float ox, float oy, float oz,
	float dx, float dy, float dz)
{
	/*
	 * The intersect context can be used to set intersection
	 * filters or flags, and it also contains the instance ID stack
	 * used in multi-level instancing.
	 */
	struct RTCIntersectContext context;
	rtcInitIntersectContext(&context);

	/*
	 * The ray hit structure holds both the ray and the hit.
	 * The user must initialize it properly -- see API documentation
	 * for rtcIntersect1() for details.
	 */
	struct RTCRayHit rayhit;
	rayhit.ray.org_x = ox;
	rayhit.ray.org_y = oy;
	rayhit.ray.org_z = oz;
	rayhit.ray.dir_x = dx;
	rayhit.ray.dir_y = dy;
	rayhit.ray.dir_z = dz;
	rayhit.ray.tnear = 0;
	rayhit.ray.tfar = INFINITY;
	rayhit.ray.mask = 0;
	rayhit.ray.flags = 0;
	rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
	rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

	/*
	 * There are multiple variants of rtcIntersect. This one
	 * intersects a single ray with the scene.
	 */
	rtcIntersect1(scene, &context, &rayhit);

	printf("%f, %f, %f: ", ox, oy, oz);
	if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
	{
		/* Note how geomID and primID identify the geometry we just hit.
		 * We could use them here to interpolate geometry information,
		 * compute shading, etc.
		 * Since there is only a single triangle in this scene, we will
		 * get geomID=0 / primID=0 for all hits.
		 * There is also instID, used for instancing. See
		 * the instancing tutorials for more information */
		printf("Found intersection on geometry %d, primitive %d at tfar=%f\n",
			rayhit.hit.geomID,
			rayhit.hit.primID,
			rayhit.ray.tfar);
	}
	else
		printf("Did not find any intersection.\n");
}


/* -------------------------------------------------------------------------- */

void SceneRenderer::RenderScene(std::pair<PointCloud, std::vector<PhotonData>>* photonMap, std::string modelName, Options options)
{
	auto cloud = photonMap->first;

	my_kd_tree_t index(3 /*dim*/, cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */));

	index.buildIndex();	

	/* Initialization. All of this may fail, but we will be notified by
	 * our errorFunction. */
	RTCDevice device = SceneLoader::initializeDevice();
	SceneInfo* sceneInfo = SceneLoader::initializeScene(device, modelName);

	/* This will hit the triangle at t=1. */
	//castRay(scene, 0, 0, -1, 0, 0, 1);

	/* This will not hit anything. */
	//castRay(scene, 1, 1, -1, 0, 0, 1);

	Renderer::render(options, sceneInfo, &index, &photonMap->second);
	//Renderer::renderPixel(35, 190, options, sceneInfo, &index, &photonMap->second);
	//Renderer::renderPixel(75, 470, options, sceneInfo, &index, &photonMap->second);
	//Renderer::renderPixel(325, 435, options, sceneInfo, &index, &photonMap->second);
	//Renderer::renderPixel(385, 396, options, sceneInfo, &index, &photonMap->second);
	//Renderer::renderPixel(360, 428, options, sceneInfo, &index, &photonMap->second);

	/* Though not strictly necessary in this example, you should
	 * always make sure to release resources allocated through Embree. */
	rtcReleaseScene(sceneInfo->scene);
	rtcReleaseDevice(device);

	//return 0;
}

