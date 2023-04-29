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

void SceneRenderer::RenderScene(Options options)
{
	/* Initialization. All of this may fail, but we will be notified by
	 * our errorFunction. */ 
	RTCDevice device = SceneLoader::initializeDevice();
	SceneInfo* sceneInfo = SceneLoader::initializeScene(device, options);

	Renderer::render(options, sceneInfo);
	//Renderer::renderPixel(35, 190, options, sceneInfo, &index, &photonMap->second);

	/* Though not strictly necessary in this example, you should
	 * always make sure to release resources allocated through Embree. */
	rtcReleaseScene(sceneInfo->scene);
	rtcReleaseDevice(device);
}

