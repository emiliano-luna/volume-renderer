#include "stdafx.h"
#include "FileManager.h"

PhotonMapOptions* FileManager::GetPhotonMapOptions()
{
	return XMLManager::GetPhotonMapOptions();
}

Options* FileManager::GetRendererOptions()
{
	return XMLManager::GetRendererOptions();
}

void FileManager::SavePhotonMap(std::pair<PointCloud, std::vector<PhotonData>>* photonMap, PhotonMapOptions* options)
{
	std::ofstream myfile;
	//myfile.open("..\\PhotonMaps\\" + options->modelName + "_" + std::to_string(options->photonCount) + ".phmap");
	myfile.open("..\\PhotonMaps\\" + options->fileName);
	myfile << options->modelName << " " << options->photonCount << std::endl;

	for (int i = 0; i < photonMap->second.size(); ++i) {
		Vec3f photon = (Vec3f)photonMap->first.pts[i];
		Vec3f color = (Vec3f)photonMap->second[i].color;

		//Guardo las coordenadas del fotón
		myfile << std::to_string(photon.x) << " " << std::to_string(photon.y) << " " << std::to_string(photon.z) << " ";
		//Guardo el color del fotón
		myfile << std::to_string(color.x) << " " << std::to_string(color.y) << " " << std::to_string(color.z);

		myfile << std::endl;
	}

	myfile.close();
}

std::pair<PointCloud, std::vector<PhotonData>>* FileManager::ReadPhotonMap(std::string name, std::string* modelName) {
	std::pair<PointCloud, std::vector<PhotonData>>* res = new std::pair<PointCloud, std::vector<PhotonData>>();

	std::ifstream infile("..\\PhotonMaps\\" + name);
	if (!infile) {
		std::cout << "Unable to open file";
		exit(1); // terminate with error
	}

	int photonCount;
	infile >> *modelName >> photonCount;

	Vec3f pos, color;
	PhotonData data;
	while (infile >> pos.x >> pos.y >> pos.z >> color.x >> color.y >> color.z)
	{
		res->first.pts.push_back(pos);

		data.color = color;
		res->second.push_back(data);
	}

	infile.close();

	return res;
}