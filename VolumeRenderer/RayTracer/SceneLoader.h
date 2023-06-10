#pragma once

#define NANOVDB_USE_ZIP

#include <Windows.h>
#include "Process.h"
#include "Utils\Types.h"
#include "Utils\Utils.h"
#include "FreeImage.h"
#include <vector>
#include <rtcore.h>
#include <stdio.h>
#include <math.h>
#include "nanovdb\nanovdb.h"
#include "nanovdb\util\IO.h"
#include "nanovdb\util\Primitives.h"

class SceneLoader
{
private:
	SceneLoader();
	static void errorFunction(void * userPtr, RTCError error, const char * str);
public:	
	static RTCDevice initializeDevice();
	static SceneInfo* initializeScene(RTCDevice device, Options options);
};

