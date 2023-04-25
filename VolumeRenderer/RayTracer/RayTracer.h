#pragma once

#ifdef RAYTRACER_EXPORTS
#define RAYTRACER_API __declspec(dllexport)
#else
#define RAYTRACER_API __declspec(dllimport)
#endif



extern "C" RAYTRACER_API void ray_tracer_test();

extern "C" RAYTRACER_API void RenderScene();

extern "C" RAYTRACER_API void GeneratePhotonMap();