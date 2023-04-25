#pragma once
#include <Windows.h>
#include "Process.h"
#include "Types.h"
#include "Utils.h"
#include "FreeImage.h"
#include <vector>
#include <rtcore.h>
#include <stdio.h>
#include <math.h>

class SceneLoader
{
private:
	SceneLoader();
	static void errorFunction(void * userPtr, RTCError error, const char * str);
public:	
	static RTCDevice initializeDevice();
	static SceneInfo* initializeScene(RTCDevice device);
};

