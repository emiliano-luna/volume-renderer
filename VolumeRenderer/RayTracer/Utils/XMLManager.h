#pragma once
#include "..\pugixml-1.9\src\pugixml.hpp"
#include "Types.h"

class XMLManager
{
private: 
	XMLManager();
public:
	static Options* GetRendererOptions();
};

