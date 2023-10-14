#pragma once
#include "XMLManager.h"
#include "Types.h"
#include <iostream>
#include <fstream>

class FileManager
{
private:
	FileManager();
public:
	static Options* GetRendererOptions();
};

