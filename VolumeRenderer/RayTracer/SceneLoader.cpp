#include "SceneLoader.h"

/*
 * We will register this error handler with the device in initializeDevice(),
 * so that we are automatically informed on errors.
 * This is extremely helpful for finding bugs in your code, prevents you
 * from having to add explicit error checking to each Embree API call.
 */
void SceneLoader::errorFunction(void* userPtr, RTCError error, const char* str)
{
	printf("error %d: %s\n", error, str);
}

/*
 * Embree has a notion of devices, which are entities that can run
 * raytracing kernels.
 * We initialize our device here, and then register the error handler so that
 * we don't miss any errors.
 *
 * rtcNewDevice() takes a configuration string as an argument. See the API docs
 * for more information.
 *
 * Note that RTCDevice is reference-counted.
 */
RTCDevice SceneLoader::initializeDevice()
{
	RTCDevice device = rtcNewDevice(NULL);

	if (!device)
		printf("error %d: cannot create device\n", rtcGetDeviceError(NULL));

	rtcSetDeviceErrorFunction(device, errorFunction, NULL);
	return device;
}

/*
 * Create a scene, which is a collection of geometry objects. Scenes are
 * what the intersect / occluded functions work on. You can think of a
 * scene as an acceleration structure, e.g. a bounding-volume hierarchy.
 *
 * Scenes, like devices, are reference-counted.
 */
SceneInfo* SceneLoader::initializeScene(RTCDevice device, Options options)
{
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	//El indice es el identificador de la primitiva y el valor es el identificador de la shape
	std::vector<unsigned int> primitives;

	auto info = new SceneInfo();

	RTCScene scene = rtcNewScene(device);

	/*
	 * Create a triangle mesh geometry, and initialize a single triangle.
	 * You can look up geometry types in the API documentation to
	 * find out which type expects which buffers.
	 *
	 * We create buffers directly on the device, but you can also use
	 * shared buffers. For shared buffers, special care must be taken
	 * to ensure proper alignment and padding. This is described in
	 * more detail in the API documentation.
	 */
	RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
	/*FileName*/

	std::vector<PointLight> lights;

	for (auto model : options.models)
	{
		std::string basedir = model.baseDir;
		std::string inputfile = basedir + model.fileName;

		tinyobj::attrib_t attrib;

		std::string warn;
		std::string err;

		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputfile.c_str(), basedir.c_str());

		if (!warn.empty()) {
			std::cout << warn << std::endl;
		}

		if (!err.empty()) {
			std::cerr << err << std::endl;
		}

		if (!ret) {
			exit(1);
		}

		std::vector<tinyobj::index_t> indexCol;


		unsigned maxVertexIndex = 0;
		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {

			// per-face material
			// we assume for performance that each face in a shape has the same material
			auto materialId = shapes[s].mesh.material_ids[0];

			bool isLight = 
				materials[materialId].emission[0] > 0 || 
				materials[materialId].emission[1] > 0 || 
				materials[materialId].emission[2] > 0;

			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				int fv = shapes[s].mesh.num_face_vertices[f];

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

					if (isLight) 
					{
						PointLight light;

						light.position = Vec3f(attrib.vertices[3 * idx.vertex_index + 0], attrib.vertices[3 * idx.vertex_index + 1], attrib.vertices[3 * idx.vertex_index + 2]);
						light.shapeIndex = s;

						lights.push_back(light);
					}
					//tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					//tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					//tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
					/*tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
					tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
					tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];*/
					//Optional: vertex colors
					//tinyobj::real_t red = attrib.colors[3 * idx.vertex_index + 0];
					//tinyobj::real_t green = attrib.colors[3 * idx.vertex_index + 1];
					//tinyobj::real_t blue = attrib.colors[3 * idx.vertex_index + 2];

					//if (idx.vertex_index > maxVertexIndex) maxVertexIndex = idx.vertex_index;
					indexCol.push_back(idx);
				}
				index_offset += fv;

				primitives.push_back(s);
			}
		}

		float* vertices = (float*)rtcSetNewGeometryBuffer(geom,
			RTC_BUFFER_TYPE_VERTEX,
			0,
			RTC_FORMAT_FLOAT3,
			3 * sizeof(float),
			attrib.vertices.size());

		unsigned* indices = (unsigned*)rtcSetNewGeometryBuffer(geom,
			RTC_BUFFER_TYPE_INDEX,
			0,
			RTC_FORMAT_UINT3,
			3 * sizeof(unsigned),
			indexCol.size() / 3);

		for (size_t i = 0; i < attrib.vertices.size(); i++)
		{
			vertices[i] = attrib.vertices[i];
		}

		int count = 0;
		for (auto i : indexCol)
		{
			indices[count] = i.vertex_index;
			count++;
		}
	}

	/*
	 * You must commit geometry objects when you are done setting them up,
	 * or you will not get any intersections.
	 */
	rtcCommitGeometry(geom);

	/*
	 * In rtcAttachGeometry(...), the scene takes ownership of the geom
	 * by increasing its reference count. This means that we don't have
	 * to hold on to the geom handle, and may release it. The geom object
	 * will be released automatically when the scene is destroyed.
	 *
	 * rtcAttachGeometry() returns a geometry ID. We could use this to
	 * identify intersected objects later on.
	 */
	rtcAttachGeometry(scene, geom);
	rtcReleaseGeometry(geom);

	/*
	 * Like geometry objects, scenes must be committed. This lets
	 * Embree know that it may start building an acceleration structure.
	 */
	rtcCommitScene(scene);

	info->scene = scene;
	info->shapes = shapes;
	info->materials = materials;
	info->primitives = primitives;
	info->lights = lights;

	if (!options.densityField.baseDir.empty()) {
		info->densityGridHandle = nanovdb::io::readGrid<nanovdb::HostBuffer>(options.densityField.baseDir + options.densityField.fileName, "density");
		info->temperatureGridHandle = nanovdb::io::readGrid<nanovdb::HostBuffer>(options.densityField.baseDir + options.densityField.fileName, "temperature");

		info->densityGrid = info->densityGridHandle.grid<float>();
		if (!info->densityGrid)
			throw std::runtime_error("GridHandle does not contain a valid host grid");

		info->temperatureGrid = info->temperatureGridHandle.grid<float>();
		if (info->temperatureGrid)
		{
			//throw std::runtime_error("handleEmission does not contain a valid host grid");
			info->temperatureExtrema = nanovdb::getExtrema(*info->temperatureGrid, info->temperatureGrid->indexBBox());
		}

		//get grid stats
		info->densityExtrema = nanovdb::getExtrema(*info->densityGrid, info->densityGrid->indexBBox());		

		float                wBBoxDimZ = (float)info->densityGrid->worldBBox().dim()[2] * 2;
		nanovdb::Vec3<float> wBBoxCenter = nanovdb::Vec3<float>(info->densityGrid->worldBBox().min() + info->densityGrid->worldBBox().dim() * 0.5f);
		nanovdb::CoordBBox treeIndexBbox = info->densityGrid->tree().bbox();

		info->gridBoundingBox = treeIndexBbox;
	}
	
	return info;
}
