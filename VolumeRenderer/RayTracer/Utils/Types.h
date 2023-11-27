#pragma once
#ifndef RAYTRACER_TYPES
#define RAYTRACER_TYPES

#define _USE_MATH_DEFINES

#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <memory>
#include <vector>
#include <utility>
#include <cstdint>
#include <fstream>
#include <limits>
#include <iostream>
#include <algorithm>
#include "rtcore.h"
#include "..\nanovdb\NanoVDB.h"
#include "..\nanovdb\util\GridHandle.h"
#include "..\tinyobjloader\tiny_obj_loader.h"
#include "../nanovdb/util/GridStats.h"
#include "RandomGenerator.h"
#include "..\nanovdb\util\Ray.h"

enum MaterialType { DIFFUSE_AND_GLOSSY, REFLECTION_AND_REFRACTION, REFLECTION };

class Types
{
public:
	Types();
	~Types();
};

class Vec2f
{
public:
	Vec2f() : x(0), y(0) {}
	Vec2f(float xx) : x(xx), y(xx) {}
	Vec2f(float xx, float yy) : x(xx), y(yy) {}
	Vec2f operator * (const float &r) const { return Vec2f(x * r, y * r); }
	Vec2f operator + (const Vec2f &v) const { return Vec2f(x + v.x, y + v.y); }
	float x, y;
};

class Vec3f {
public:
	Vec3f() : x(0), y(0), z(0) {}
	Vec3f(float xx) : x(xx), y(xx), z(xx) {}
	Vec3f(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}
	Vec3f(float* xyz) : x(xyz[0]), y(xyz[1]), z(xyz[2]) {}
	Vec3f operator * (const float &r) const { return Vec3f(x * r, y * r, z * r); }
	Vec3f operator * (const Vec3f &v) const { return Vec3f(x * v.x, y * v.y, z * v.z); }
	Vec3f operator - (const Vec3f &v) const { return Vec3f(x - v.x, y - v.y, z - v.z); }
	Vec3f operator + (const Vec3f &v) const { return Vec3f(x + v.x, y + v.y, z + v.z); }
	Vec3f operator - () const { return Vec3f(-x, -y, -z); }
	Vec3f& operator += (const Vec3f &v) { x += v.x, y += v.y, z += v.z; return *this; }
	Vec3f& operator *= (const Vec3f &v) { x *= v.x, y *= v.y, z *= v.z; return *this; }
	bool operator == (const float xyz) { return xyz == x && xyz == y && xyz == z; }
	friend Vec3f operator * (const float &r, const Vec3f &v)
	{
		return Vec3f(v.x * r, v.y * r, v.z * r);
	}
	friend Vec3f operator / (const Vec3f& v, const int& r)
	{
		return Vec3f(v.x / r, v.y / r, v.z / r);
	}
	friend std::ostream & operator << (std::ostream &os, const Vec3f &v)
	{
		return os << v.x << ", " << v.y << ", " << v.z;
	}
	float x, y, z;
};

struct Model {
	std::string baseDir;
	std::string fileName;
};

struct Options
{	
	std::vector<Model> models;
	Model densityField;
	float sigma_s;
	float sigma_a;
	float sigma_n;
	uint32_t rayPerPixelCount;
	uint32_t width;
	uint32_t widthStartOffset;
	uint32_t widthReference;
	uint32_t height;
	uint32_t heightStartOffset;
	uint32_t heightReference;
	float fov;
	float imageAspectRatio;
	uint8_t maxDepth;
	Vec3f backgroundColor;
	Vec3f ambientLight;
	bool multiThreaded;
	uint32_t multiThreadedChunkSize;
	std::string fileName;
	Vec3f cameraPosition;
	Vec3f cameraRotation;	
	std::string integrator;
	Vec3f lightPosition;
	Vec3f lightColor;
	Vec3f emissionColor;
	float heyneyGreensteinG;
	bool useImportanceSampling;
	float stepSizeMin;
	float stepSizeMax;
};

struct PointLight {
	Vec3f position;
	size_t shapeIndex;
};

struct SceneInfo {
	RTCScene scene;
	std::vector<unsigned int> primitives;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	/// <summary>
	/// for now a single area light is represented as multiple point lights at its vertices
	/// </summary>
	std::vector<PointLight> lights;
	//don't remove this handles, I need to keep them alive or density/temperature grids won't work
	nanovdb::GridHandle<nanovdb::HostBuffer> densityGridHandle;
	nanovdb::GridHandle<nanovdb::HostBuffer> temperatureGridHandle;
	nanovdb::FloatGrid* densityGrid;
	nanovdb::FloatGrid* temperatureGrid;
	nanovdb::Extrema<float> densityExtrema;
	nanovdb::Extrema<float> temperatureExtrema;
	nanovdb::CoordBBox gridBoundingBox;
};

class ThreadInfo {
public:
	uint32_t fromHeight;
	uint32_t toHeight;
};

#endif // !RAYTRACER_TYPES