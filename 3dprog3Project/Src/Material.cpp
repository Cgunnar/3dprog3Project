#include "pch.h"
#include "Material.h"
#include "AssetManager.h"

void Material::SetAlbedoTexture(const std::string& path)
{
	albedoID = AssetManager::Get().AddTextureFromFile(path, false, false);
}

void Material::SetNormalTexture(const std::string& path)
{

}

void Material::SetMetallicRoughnessTexture(const std::string& path)
{

}

void Material::SetEmissiveTexture(const std::string& path)
{

}
