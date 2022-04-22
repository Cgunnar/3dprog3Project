#pragma once

#include "Mesh.h"
#include "Material.h"

class AssetManager
{
public:
	static void Init();
	static void Destroy();
	static AssetManager& Get();

	uint64_t AddMesh(const Mesh& mesh);
	uint64_t AddMaterial(const Material &material);

	std::shared_ptr<Mesh> GetMesh(uint64_t id) const;
	std::shared_ptr<Material> GetMaterial(uint64_t id) const;

	void RemoveMesh(uint64_t id);
	void RemoveMaterial(uint64_t id);

private:
	AssetManager();
	~AssetManager();
	AssetManager(const AssetManager& other) = delete;
	AssetManager& operator=(const AssetManager& other) = delete;

	static AssetManager* s_instance;

	std::unordered_map<uint64_t, std::shared_ptr<Mesh>> m_meshes;
	std::unordered_map<uint64_t, std::shared_ptr<Material>> m_materials;
};