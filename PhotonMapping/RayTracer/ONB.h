#include "Utils.h"

class ONB {

public:
	ONB(Vec3f normal);
	Vec3f WorldToLocal(const Vec3f &v);
	Vec3f LocalToWorld(const Vec3f &v);
	Vec3f s;
	Vec3f t;
private:
	Vec3f n;
};