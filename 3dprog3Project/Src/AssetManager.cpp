#include "pch.h"
#include "AssetManager.h"
#include "FrameTimer.h"

AssetManager* AssetManager::s_instance = nullptr;

void AssetManager::Init(ID3D12Device* device)
{
	assert(!s_instance);
	s_instance = new AssetManager(device);
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
	if (m_meshes.contains(id))
	{
		m_gpuAssetsToRemove.emplace(std::make_pair(m_meshes[id].vertexBuffer, FrameTimer::Frame()));
		m_gpuAssetsToRemove.emplace(std::make_pair(m_meshes[id].indexBuffer, FrameTimer::Frame()));
		m_meshes.erase(id);
	}
}

void AssetManager::RemoveMaterial(uint64_t id)
{
	if (m_materials.contains(id))
	{
		m_gpuAssetsToRemove.emplace(std::make_pair(m_materials[id].constantBuffer, FrameTimer::Frame()));
		m_materials.erase(id);
	}
}

void AssetManager::Update(int numberOfFramesInFlight)
{
	bool keepRemoving = true;
	while (!m_gpuAssetsToRemove.empty() && keepRemoving)
	{
		auto[asset, frame] = m_gpuAssetsToRemove.front();
		if (!asset.resource)
		{
			m_gpuAssetsToRemove.pop(); //resource was never sent to the gpu
		}
		else if(asset.resource && FrameTimer::Frame() - frame > numberOfFramesInFlight)
		{
			asset.resource->Release();
			m_gpuAssetsToRemove.pop();
		}
		else
		{
			keepRemoving = false;
		}
	}
}

const DescriptorVector& AssetManager::GetHeapDescriptors() const
{
	return m_heapDescriptor;
}

AssetManager::AssetManager(ID3D12Device* device)
{
	m_heapDescriptor.Init(device);
}

AssetManager::~AssetManager()
{
	while (!m_gpuAssetsToRemove.empty())
	{
		if (m_gpuAssetsToRemove.front().first.resource)
			m_gpuAssetsToRemove.front().first.resource->Release();
		m_gpuAssetsToRemove.pop();
	}
}
