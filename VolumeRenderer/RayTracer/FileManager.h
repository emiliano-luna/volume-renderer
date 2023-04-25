#pragma once
#include "XMLManager.h"
#include "nanonflann\nanoflann.hpp"
#include "nanonflann\utils.h"
#include "Types.h"
#include <iostream>
#include <fstream>

class FileManager
{
private:
	FileManager();
public:
	static PhotonMapOptions* GetPhotonMapOptions();
	static Options* GetRendererOptions();
	static void SavePhotonMap(std::pair<PointCloud, std::vector<PhotonData>>* photonMap, PhotonMapOptions* options);
	static std::pair<PointCloud, std::vector<PhotonData>>* ReadPhotonMap(std::string name, std::string* modelName);
};

