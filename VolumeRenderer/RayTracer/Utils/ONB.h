#ifndef VOLUMERENDERER_ONB
#define VOLUMERENDERER_ONB

#include "Utils.h"

//orthonormal-basis
class ONB {

public:
	ONB();
	void Update(Vec3f normal);
	Vec3f WorldToLocal(const Vec3f &v);
	Vec3f LocalToWorld(const Vec3f &v);
	Vec3f s;
	Vec3f t;
private:
	Vec3f n;
};
#endif