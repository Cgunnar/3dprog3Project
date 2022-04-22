#pragma once

#include "Mesh.h"
#include "Material.h"

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
	static void Init();
	static void Destroy();
	static AssetManager& Get();

	uint64_t AddMesh(const Mesh& mesh);
	uint64_t AddMaterial(const Material &material);

	MeshAsset GetMesh(uint64_t id) const;
	MaterialAsset GetMaterial(uint64_t id) const;

	void RemoveMesh(uint64_t id);
	void RemoveMaterial(uint64_t id);

private:
	AssetManager();
	~AssetManager();
	AssetManager(const AssetManager& other) = delete;
	AssetManager& operator=(const AssetManager& other) = delete;

	static AssetManager* s_instance;

	std::unordered_map<uint64_t, MeshAsset> m_meshes;
	std::unordered_map<uint64_t, MaterialAsset> m_materials;
};