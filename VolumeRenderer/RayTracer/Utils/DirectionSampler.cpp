#include "DirectionSampler.h"
#include <algorithm>

DirectionSampler::DirectionSampler() {
	onb = new ONB();
}

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

//g parámetro de anisotropía (g=0 isotrópico; g>0 anisotropía hacia adelante; g<0 anisotropía hacia atrás)
Vec3f DirectionSampler::sampleHenyeyGreenstein(float g, Vec3f direction, RandomGenerator* generator) {
	onb->Update(direction);

	auto normalizedDirection = Utils::normalize(direction);
	float cos_theta;
	float xi = generator->getFloat(0.0f, 0.9999f);
	 
	if (g != 0.0f) {
		//get random theta and phi
		//theta using Henyey-Greenstein function
		float aux = ((1 - g * g) / (1 - g + 2 * g * xi));
		cos_theta = (1 + g * g - (aux * aux)) / (2 * g);
	}
	else {
		cos_theta = 1.0 - 2.0 * xi;
	}

	float sin_theta = sqrt(std::max(0.0f, 1.0f - cos_theta * cos_theta));

	//phi randomly with uniform distribution in [0, 2*pi0]
	float phi = generator->getFloat(0.0f, 0.9999f) * 2 * M_PI;

	//(0,0,1) when gHG = 1.0
	auto localDirection = Vec3f(
		sin_theta * cos(phi),
		sin_theta * sin(phi),
		cos_theta);	

	auto v1 = localDirection.x * onb->s;
	auto v2 = localDirection.y * onb->t;
	auto v3 = localDirection.z * normalizedDirection;
	auto v  = v1 + v2 + v3;

	return v;
}