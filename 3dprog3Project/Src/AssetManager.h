#pragma once

#include "Mesh.h"
#include "Material.h"
#include "DescriptorVector.h"

struct GPUAsset
{
	ID3D12Resource* resource = nullptr;
	UINT elementSize = 0;
	UINT elementCount = 0;
	UINT descIndex = -1;
	bool valid = false;
	bool cpuMappable = false;
};

struct MeshAsset
{
	std::shared_ptr<Mesh> mesh;
	GPUAsset vertexBuffer;
	GPUAsset indexBuffer;
};

struct MaterialAsset
{
	std::shared_ptr<Material> material;
	GPUAsset constantBuffer;
};

class AssetManager
{
public:
	static void Init(ID3D12Device* device);
	static void Destroy();
	static AssetManager& Get();

	uint64_t AddMesh(const Mesh& mesh);
	uint64_t AddMaterial(const Material &material);

	MeshAsset GetMesh(uint64_t id) const;
	MaterialAsset GetMaterial(uint64_t id) const;

	void RemoveMesh(uint64_t id);
	void RemoveMaterial(uint64_t id);

	const DescriptorVector& GetHeapDescriptors() const;

private:
	AssetManager(ID3D12Device* device);
	~AssetManager();
	AssetManager(const AssetManager& other) = delete;
	AssetManager& operator=(const AssetManager& other) = delete;

	static AssetManager* s_instance;

	std::unordered_map<uint64_t, MeshAsset> m_meshes;
	std::unordered_map<uint64_t, MaterialAsset> m_materials;
	DescriptorVector m_heapDescriptor = DescriptorVector(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
};