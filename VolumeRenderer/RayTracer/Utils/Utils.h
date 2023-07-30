#ifndef RAYTRACER_UTILS
#define RAYTRACER_UTILS

#pragma once

#include "Types.h"
#include <ctime>    // For time()
#include <cstdlib>  // For srand() and rand()
#include <random>
#include <functional>

class Utils
{
private:
	Utils();
public:
	static Vec3f crossProduct(const Vec3f &a, const Vec3f &b);
	static Vec3f normalize(const Vec3f &v);
	static void rotate(float pitch, float roll, float yaw, Vec3f * point);
	static float dotProduct(const Vec3f & a, const Vec3f & b);
	static float clamp(const float & lo, const float & hi, const float & v);
	static float deg2rad(const float & deg);
	static bool solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1);
	static float distance2(const Vec3f &a, const Vec3f &b);
	/*static float getRandomFloat(float min, float max);*/
	static Vec3f reflect(const Vec3f &I, const Vec3f &N);
	Vec3f mix(const Vec3f & a, const Vec3f & b, const float & mixValue);
	~Utils();
};

#endif // !1