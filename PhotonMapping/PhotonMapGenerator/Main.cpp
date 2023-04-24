//#define NOMINMAX
#include <iostream>
#include "..\RayTracer\RayTracer.h"

// In the main function of the program, we create the scene (create objects and lights)
// as well as set the options for the render (image widht and height, maximum recursion
// depth, field-of-view, etc.). We then call the render function().
int main(int argc, char **argv)
{
	//PhotonMapOptions options = GetPhotonMapOptions();

	//ray_tracer_test();
	GeneratePhotonMap();

	std::getchar();
}