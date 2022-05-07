#pragma once
#include "RimfrostMath.hpp"
class Material
{
public:
	rfm::Vector4 albedoFactor = { 1,1,1,1 };
	rfm::Vector4 emissionFactor = { 0,0,0,0 };
	uint64_t albedoID = 0;
private:
};

