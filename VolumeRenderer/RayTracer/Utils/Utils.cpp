#include "Utils.h"

Vec3f Utils::crossProduct(const Vec3f &a, const Vec3f &b)
{
	return Vec3f(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	);
}

Vec3f Utils::normalize(const Vec3f &v)
{
	float mag2 = v.x * v.x + v.y * v.y + v.z * v.z;
	if (mag2 > 0) {
		float invMag = 1 / sqrtf(mag2);
		return Vec3f(v.x * invMag, v.y * invMag, v.z * invMag);
	}

	return v;
}

void Utils::rotate(float pitch, float roll, float yaw, Vec3f *point) {
	pitch = (pitch * M_PI) / 180;
	roll = (roll * M_PI) / 180;
	yaw = (yaw * M_PI) / 180;

	float cosa = cos(yaw);
	float sina = sin(yaw);

	float cosb = cos(pitch);
	float sinb = sin(pitch);

	float cosc = cos(roll);
	float sinc = sin(roll);

	float Axx = cosa * cosb;
	float Axy = cosa * sinb*sinc - sina * cosc;
	float Axz = cosa * sinb*cosc + sina * sinc;

	float Ayx = sina * cosb;
	float Ayy = sina * sinb*sinc + cosa * cosc;
	float Ayz = sina * sinb*cosc - cosa * sinc;

	float Azx = -sinb;
	float Azy = cosb * sinc;
	float Azz = cosb * cosc;

	//for (int i = 0; i < points.length; i++) {
	float px = point->x;//points[i].x;
	float py = point->y;
	float pz = point->z;

	point->x = Axx * px + Axy * py + Axz * pz;
	point->y = Ayx * px + Ayy * py + Ayz * pz;
	point->z = Azx * px + Azy * py + Azz * pz;
	//}
}

// Compute reflection direction
Vec3f Utils::reflect(const Vec3f &I, const Vec3f &N)
{
	return I - 2 * Utils::dotProduct(I, N) * N;
}

float Utils::dotProduct(const Vec3f &a, const Vec3f &b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float Utils::clamp(const float &lo, const float &hi, const float &v)
{
	return std::max(lo, std::min(hi, v));
}

float Utils::deg2rad(const float &deg)
{
	return deg * M_PI / 180;
}

float Utils::getRandomFloat(float low, float high, unsigned int seed) {
	srand(seed);

	return low + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (high - low)));
}

inline
Vec3f Utils::mix(const Vec3f &a, const Vec3f& b, const float &mixValue)
{
	return a * (1 - mixValue) + b * mixValue;
}

float Utils::distance2(const Vec3f &a, const Vec3f& b) {
	return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z);
}

bool Utils::solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1)
{
	float discr = b * b - 4 * a * c;
	if (discr < 0) return false;
	else if (discr == 0) x0 = x1 = -0.5 * b / a;
	else {
		float q = (b > 0) ?
			-0.5 * (b + sqrt(discr)) :
			-0.5 * (b - sqrt(discr));
		x0 = q / a;
		x1 = c / q;
	}
	if (x0 > x1) std::swap(x0, x1);
	return true;
}

Utils::~Utils()
{
}
