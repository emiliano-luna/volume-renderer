#pragma once
#ifndef VOLUMERENDERER_INTEGRATORFACTORY
#define VOLUMERENDERER_INTEGRATORFACTORY

#include <string>
#include "BaseIntegrator.h"

class IntegratorFactory
{
public:
	static BaseIntegrator* GetIntegrator(std::string name);
};
#endif 
