#include "PhotonMapper.h"

/*
 * Cast a single ray with origin (ox, oy, oz) and direction
 * (dx, dy, dz).
 */
RTCRayHit PhotonMapper::castRay(RTCScene scene,
	Vec3f origin,
	Vec3f direction)
{
	struct RTCRayHit rayhit;

	try
	{
		/*
		 * The intersect context can be used to set intersection
		 * filters or flags, and it also contains the instance ID stack
		 * used in multi-level instancing.
		 */
		struct RTCRayQueryContext context;
		rtcInitRayQueryContext(&context);

		/*
		 * The ray hit structure holds both the ray and the hit.
		 * The user must initialize it properly -- see API documentation
		 * for rtcIntersect1() for details.
		 */
	
		rayhit.ray.org_x = origin.x;
		rayhit.ray.org_y = origin.y;
		rayhit.ray.org_z = origin.z;
		rayhit.ray.dir_x = direction.x;
		rayhit.ray.dir_y = direction.y;
		rayhit.ray.dir_z = direction.z;
		rayhit.ray.tnear = 0;
		rayhit.ray.tfar = INFINITY;
		rayhit.ray.mask = 0;
		rayhit.ray.flags = 0;
		rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
		rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

		/*
		 * There are multiple variants of rtcIntersect. This one
		 * intersects a single ray with the scene.
		 */
		rtcIntersect1(scene, &rayhit);

		//printf("%f, %f, %f: ", ox, oy, oz);
		if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
		{
			/* Note how geomID and primID identify the geometry we just hit.
			 * We could use them here to interpolate geometry information,
			 * compute shading, etc.
			 * Since there is only a single triangle in this scene, we will
			 * get geomID=0 / primID=0 for all hits.
			 * There is also instID, used for instancing. See
			 * the instancing tutorials for more information */
			//printf("Found intersection on geometry %d, primitive %d at tfar=%f\n",
			//	rayhit.hit.geomID,
			//	rayhit.hit.primID,
			//	rayhit.ray.tfar);
		}
		else {}
			/*printf("Did not find any intersection.\n");*/
	}
	catch (const std::exception&)
	{

	}

	return rayhit;
}

std::pair<PointCloud, std::vector<PhotonData>>* PhotonMapper::Generate(PhotonMapOptions* options) {
	/* Initialization. All of this may fail, but we will be notified by
	 * our errorFunction. */
	RTCDevice device = SceneLoader::initializeDevice();
	SceneInfo* sceneInfo = SceneLoader::initializeScene(device, options->modelName);

	//TO DO: Guardar Archivo y no retornar
	auto res = PhotonMapper::GeneratePhotons(sceneInfo, options);

	/* Though not strictly necessary in this example, you should
	 * always make sure to release resources allocated through Embree. */
	rtcReleaseScene(sceneInfo->scene);
	rtcReleaseDevice(device);

	return res;
}

std::pair<PointCloud, std::vector<PhotonData>>* PhotonMapper::GeneratePhotons(SceneInfo* sceneInfo, PhotonMapOptions* options) {
	auto res = new std::pair<PointCloud, std::vector<PhotonData>>();
	PointCloud cloud;
	std::vector<PhotonData> data;

	// Generate points:
	//generateRandomPointCloud(cloud, 1000);

	srand(time(0));  // Initialize random number generator.

	auto photonColor = options->lightColor * ((float)1 / options->photonCount);
	//auto photonColor = options->lightColor;
	
	for (size_t i = 0; i < options->photonCount; i++)
	{
		float x, y, z;

		do {
			//use simple rejection sampling to find diffuse photon direction
			x = Utils::getRandomFloat(-1, 1);
			y = Utils::getRandomFloat(-1, 0);
			z = Utils::getRandomFloat(-1, 1);
		} while (x*x + y * y + z * z > 1);

		Vec3f d = Vec3f(x, y, z);

		//Vec3f p = Vec3f(0, 1.5, 0);
		Vec3f p = options->lightPos;

		PhotonMapper::GeneratePhotonRay(p, d, photonColor, 5, &cloud, &data, sceneInfo);
		//addPointToPointCloud()
	}

	// construct a kd-tree index:
	//typedef nanoflann::KDTreeSingleIndexAdaptor<
	//	nanoflann::L2_Simple_Adaptor<PointCoord, PointCloud>,
	//	PointCloud,
	//	3 /* dim */
	//> my_kd_tree_t;

	//my_kd_tree_t index(3 /*dim*/, cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */));
	//index.buildIndex();	

	//index.
	res->first = cloud;
	res->second = data;

	return res;
}

Vec3f PhotonMapper::getRebound(Vec3f normal) {
	auto onb = new ONB(normal);

	auto r1 = Utils::getRandomFloat(0, 1);
	auto r2 = Utils::getRandomFloat(0, 1);

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
	//	vel = []
	Vec3f res;
	res = v1 + v2 + v3;

	return res;
}

void PhotonMapper::GeneratePhotonRay(Vec3f origin, Vec3f direction, Vec3f photonColor, unsigned reboundLimit, PointCloud* cloud, std::vector<PhotonData>* data, SceneInfo* sceneInfo) {
	/*p~= light source position
	trace photon from ~p in direction ~
	d
	ne = ne + 1*/
	
	//scale power of stored photons wi

	auto rayHit = castRay(sceneInfo->scene, origin, direction);

	if (rayHit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
	{
		auto hitPosition = direction * rayHit.ray.tfar + origin;
		auto shapeIndex = sceneInfo->primitives[rayHit.hit.primID];
		auto material = sceneInfo->materials[sceneInfo->shapes[shapeIndex].mesh.material_ids[0]];

		//Ruleta rusa
		//Es un material difuso
		if (material.diffuse[0] != 0 && material.diffuse[1] != 0 && material.diffuse[2] != 0)
		{
			cloud->pts.push_back(hitPosition);
			PhotonData phData;
			phData.color = photonColor;
			data->push_back(phData);

			auto Pd = std::max({ photonColor.x * material.diffuse[0], photonColor.y * material.diffuse[1], photonColor.z * material.diffuse[2] }) /
					  std::max({ photonColor.x, photonColor.y, photonColor.z});

			//Absorbo
			if (Utils::getRandomFloat(0, 1) >= Pd) {
			}
			//Reboto
			else
			{
				//Si el rayo puede rebotar
				if (reboundLimit > 0)
				{
					auto dirRebound = getRebound(Vec3f(rayHit.hit.Ng_x, rayHit.hit.Ng_y, rayHit.hit.Ng_z));
					//dirRebound = Utils::normalize(dirRebound);

					Vec3f colorRebound = photonColor;
					colorRebound.x = (colorRebound.x * material.diffuse[0]) / Pd;
					colorRebound.y = (colorRebound.y * material.diffuse[1]) / Pd;
					colorRebound.z = (colorRebound.z * material.diffuse[2]) / Pd;

					GeneratePhotonRay(hitPosition + dirRebound * 0.001, dirRebound, colorRebound, reboundLimit - 1, cloud, data, sceneInfo);
				}
			}
		}
		//Es un material especular
		else {
			auto Ps = std::max({ photonColor.x * material.specular[0], photonColor.y * material.specular[1], photonColor.z * material.specular[2] }) /
					  std::max({ photonColor.x, photonColor.y, photonColor.z });

			//Absorbo
			if (Utils::getRandomFloat(0, 1) >= Ps) {
			}
			//Reboto
			else
			{
				//Si el rayo puede rebotar
				if (reboundLimit > 0)
				{
					auto normal = Utils::normalize(Vec3f(rayHit.hit.Ng_x, rayHit.hit.Ng_y, rayHit.hit.Ng_z));

					auto newDir = Utils::normalize(Utils::reflect(direction, normal));

					Vec3f colorRebound = photonColor;
					colorRebound.x = (colorRebound.x * material.specular[0]) / Ps;
					colorRebound.y = (colorRebound.y * material.specular[1]) / Ps;
					colorRebound.z = (colorRebound.z * material.specular[2]) / Ps;

					//hitColor = castRay(hitPoint, newDir, scene, objectId, options, depth + 1, photons, photonData);
					GeneratePhotonRay(hitPosition + newDir * 0.001, newDir, photonColor, reboundLimit - 1, cloud, data, sceneInfo);
				}
			}
		}
	}	
}
