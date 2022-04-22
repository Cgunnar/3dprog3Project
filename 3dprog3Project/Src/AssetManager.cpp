#include "pch.h"
#include "AssetManager.h"

AssetManager* AssetManager::s_instance = nullptr;

void AssetManager::Init()
{
	assert(!s_instance);
	s_instance = new AssetManager();
}

void AssetManager::Destroy()
{
	assert(s_instance);
	delete s_instance;
	s_instance = nullptr;
}

AssetManager& AssetManager::Get()
{
	assert(s_instance);
	return *s_instance;
}

uint64_t AssetManager::AddMesh(const Mesh& mesh)
{
	uint64_t id = utl::GenerateRandomID();
	
	MeshAsset asset;
	asset.mesh = std::make_shared<Mesh>(mesh);
	m_meshes[id] = asset;
	return id;
}

uint64_t AssetManager::AddMaterial(const Material& material)
{
	uint64_t id = utl::GenerateRandomID();
	MaterialAsset asset;
	asset.material = std::make_shared<Material>(material);
	m_materials[id] = asset;
	return id;
}

MeshAsset AssetManager::GetMesh(uint64_t id) const
{
	if (m_meshes.contains(id))
	{
		return m_meshes.at(id);
	}
	else
	{
		return MeshAsset();
	}
}

MaterialAsset AssetManager::GetMaterial(uint64_t id) const
{
	if (m_materials.contains(id))
	{
		return m_materials.at(id);
	}
	else
	{
		return MaterialAsset();
	}
}

void AssetManager::RemoveMesh(uint64_t id)
{
	m_meshes.erase(id);
}

void AssetManager::RemoveMaterial(uint64_t id)
{
	m_materials.erase(id);
}

AssetManager::AssetManager()
{

}

AssetManager::~AssetManager()
{

}
