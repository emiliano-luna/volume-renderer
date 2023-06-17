#pragma once
#include "stdafx.h"
#include "SceneRenderer.h"
#include "renderers/RendererFactory.h"
#include "renderers/BaseRenderer.h"

#if defined(RTC_NAMESPACE_OPEN)
RTC_NAMESPACE_OPEN
#endif

#include "SceneLoader.h"

void SceneRenderer::RenderScene(Options options)
{
	/* Initialization. All of this may fail, but we will be notified by
	 * our errorFunction. */ 
	RTCDevice device = SceneLoader::initializeDevice();
	SceneInfo* sceneInfo = SceneLoader::initializeScene(device, options);

	BaseRenderer* renderer = RendererFactory::GetRenderer(options.renderer);

	renderer->render(options, sceneInfo);
	//Renderer::renderPixel(320, 345, options, sceneInfo);

	/* Though not strictly necessary in this example, you should
	 * always make sure to release resources allocated through Embree. */
	rtcReleaseScene(sceneInfo->scene);
	rtcReleaseDevice(device);
}

