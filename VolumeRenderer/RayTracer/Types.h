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
#include "tinyobjloader\tiny_obj_loader.h"

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
	uint32_t width;
	uint32_t height;
	float fov;
	float imageAspectRatio;
	uint8_t maxDepth;
	Vec3f backgroundColor;
	Vec3f ambientLight;
	float bias;
	bool multiThreaded;
	bool antiAliasing;
	float colorDiffThreshold;
	bool auxImages;
	float radiusSearch;
	std::string fileName;
	Vec3f cameraPosition;
	Vec3f cameraRotation;	
	std::string intersectionHandler;
};

struct SceneInfo {
	RTCScene scene;
	std::vector<unsigned int> primitives;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
};

struct PhotonData {
	Vec3f color;
};
//struct PhotonMap {
//	PointCould* cloud;
//	std::vector<PhotonData> data;
//};

#endif // !RAYTRACER_TYPES