#include "DirectionSampler.h"

ONB* DirectionSampler::onb = new ONB();

Vec3f DirectionSampler::getCosineDistributionRebound(Vec3f normal, RandomGenerator *generator) {
	onb->Update(normal);

	auto r1 = generator->getFloat(0, 1);
	auto r2 = generator->getFloat(0, 1);

	auto sin_theta = sqrt(r1);
	auto cos_theta = sqrt(1 - sin_theta * sin_theta);

	//random in plane angle
	auto psi = r2 * 2 * M_PI;

	//three vector components
	auto a = sin_theta * cos(psi);
	auto b = sin_theta * sin(psi);
	auto c = cos_theta;

	//multiply by corresponding directions	
	auto v1 = a * onb->s;
	auto v2 = b * onb->t;
	auto v3 = c * normal;

	//add up to get velocity, vel = v1 + v2 + v3
	Vec3f res;
	res = v1 + v2 + v3;

	return res;
}