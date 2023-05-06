#pragma once
#include "..\pugixml-1.9\src\pugixml.hpp"
#include "..\Options.h"
#include "Types.h"

class XMLManager
{
private: 
	XMLManager();
public:
	static PhotonMapOptions* GetPhotonMapOptions();	
	static Options* GetRendererOptions();
};

