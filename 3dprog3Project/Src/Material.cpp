#include "pch.h"
#include "Material.h"
#include "AssetManager.h"

void Material::SetAlbedoTexture(const std::string& path)
{
	albedoID = AssetManager::Get().AddTextureFromFile(path, TextureType::albedo, true, false);
}

void Material::SetNormalTexture(const std::string& path)
{
	normalID = AssetManager::Get().AddTextureFromFile(path, TextureType::normalMap, true, true);
}

void Material::SetMetallicRoughnessTexture(const std::string& path)
{
	metallicRoughnessID = normalID = AssetManager::Get().AddTextureFromFile(path, TextureType::metallicRoughness, true, false);
}

void Material::SetEmissiveTexture(const std::string& path)
{
	//emissiveID = AssetManager::Get().AddTextureFromFile(path, TextureType::emissive, false, false);
}
