#pragma once
#include "RimfrostMath.hpp"
class Material
{
public:
	rfm::Vector4 albedoFactor = { 1,1,1,1 };
	rfm::Vector3 emissiveFactor = { 0,0,0 };
	float metallicFactor = 0;
	float roughnessFactor = 1;
	uint64_t albedoID = 0;
	uint64_t normalID = 0;
	uint64_t metallicRoughnessID = 0;
	uint64_t emissiveID = 0;
	std::string name;

	void SetAlbedoTexture(const std::string& path);
	void SetNormalTexture(const std::string& path);
	void SetMetallicRoughnessTexture(const std::string& path);
	void SetEmissiveTexture(const std::string& path);
};

