#include "pch.h"
#include "Material.h"
#include "AssetManager.h"
#include <stb_image.h>

void Material::SetAlbedoTexture(const std::string& path)
{
	albedoID = AssetManager::Get().AddTextureFromFile(path, TextureType::albedo, false, false);
}

void Material::SetNormalTexture(const std::string& path)
{
	normalID = AssetManager::Get().AddTextureFromFile(path, TextureType::normalMap, false, true);
}

void Material::SetMetallicRoughnessTexture(const std::string& path)
{
	AssetManager::Image image = AssetManager::LoadImageFromFile(path);
	const unsigned char ao = 255;
	bool setRedToOne = true;
	int sum = 0;
	for (int i = 0; i < image.height * image.width * image.bytePerPixel; i+=4)
	{
		sum += image.dataPtr[i];
	}
	double avg = static_cast<double>(sum) / (image.height * image.width);
	if (avg < 10) // if a ao texture is in the red channel the avg would be higher
	{
		for (int i = 0; i < image.height * image.width * image.bytePerPixel; i += 4)
		{
			image.dataPtr[i] = ao;
		}
	}
	else
	{
		utl::PrintDebug("ao channel exists in file:" + path);
	}
	metallicRoughnessID = AssetManager::Get().AddTextureFromMemory(image, TextureType::metallicRoughness, false, false);
	stbi_image_free(image.dataPtr);
}

void Material::SetEmissiveTexture(const std::string& path)
{
	//emissiveID = AssetManager::Get().AddTextureFromFile(path, TextureType::emissive, false, false);
}
