#pragma once
#include "stdafx.h"

#include <rtcore.h>
#include <stdio.h>
#include <math.h>

class MinimalTutorial
{
private:
	static void errorFunction(void * userPtr, RTCError error, const char * str);
	static RTCDevice initializeDevice();
	static RTCScene initializeScene(RTCDevice device);
	static void castRay(RTCScene scene, float ox, float oy, float oz, float dx, float dy, float dz);
public:
	static int main2();
};

