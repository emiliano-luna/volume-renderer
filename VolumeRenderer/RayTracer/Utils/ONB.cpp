#include "ONB.h"

ONB::ONB() {}

void ONB::Update(Vec3f normal) {
	n = normal;

	if (fabs(n.x) > fabs(n.z)) {
		s.x = -n.y;
		s.y = n.x;
		s.z = 0;
	}
	else {

		s.x = 0;
		s.y = -n.z;
		s.z = n.y;
	}

	s = Utils::normalize(s);
	t = Utils::crossProduct(n, s);
}

Vec3f ONB::WorldToLocal(const Vec3f &v) {
	return Vec3f(Utils::dotProduct(v, s), Utils::dotProduct(v, t), Utils::dotProduct(v, n));
}

Vec3f ONB::LocalToWorld(const Vec3f &v) {
	return  (v.x * s) + (v.y * t) + (v.z * n);
}