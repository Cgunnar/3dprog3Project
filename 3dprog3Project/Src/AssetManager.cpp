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
	m_meshes[id] = std::make_shared<Mesh>(mesh);
	return id;
}

uint64_t AssetManager::AddMaterial(const Material& material)
{
	uint64_t id = utl::GenerateRandomID();
	m_materials[id] = std::make_shared<Material>(material);
	return id;
}

std::shared_ptr<Mesh> AssetManager::GetMesh(uint64_t id) const
{
	if (m_meshes.contains(id))
	{
		return m_meshes.at(id);
	}
	else
	{
		return nullptr;
	}
}

std::shared_ptr<Material> AssetManager::GetMaterial(uint64_t id) const
{
	if (m_materials.contains(id))
	{
		return m_materials.at(id);
	}
	else
	{
		return nullptr;
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
