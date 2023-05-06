#pragma once
#include "XMLManager.h"
#include "..\nanonflann\nanoflann.hpp"
#include "..\nanonflann\utils.h"
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
};

