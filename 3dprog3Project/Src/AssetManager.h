#pragma once

#include "Mesh.h"
#include "Material.h"
#include "DescriptorVector.h"
#include "Renderer.h"


struct GPUAsset
{
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	UINT elementSize = 0;
	UINT elementCount = 0;
	UINT descIndex = -1;
	enum Flag
	{
		NONE = 0,
		CBV = 1 << 1,
		SRV = 1 << 2,
		UAV = 1 << 3,
		CPU_WRITE = 1 << 4,
		BUFFER = 1 << 5,
		TEXTURE_2D = 1 << 6,
	};
	Flag flag = Flag::NONE;
	bool valid = false;
};

struct SubMesh
{
	uint32_t indexStart = 0;
	uint32_t indexCount = 0;
	uint32_t vertexStart = 0;
	uint32_t vertexCount = 0;
	uint64_t materialID = 0;
	uint64_t subMeshID = 0;
};

struct SubMeshes
{
	SubMeshes(const std::vector<SubMesh>& submeshes = {}) : subMeshes(submeshes){}
	uint64_t meshAssetID = 0;
	std::vector<SubMesh> subMeshes;
};

struct MeshAsset
{
	MeshAsset() = default;
	MeshAsset(const Mesh& mesh, bool inludeInAccelerationStructure, const std::optional<SubMeshes>& subMeshes = std::nullopt)
		: inludedInAccelerationStructure(inludeInAccelerationStructure), subMeshes(subMeshes)
	{
		this->mesh = std::make_shared<Mesh>(mesh);
		vertexBuffer.elementCount = mesh.GetVertexCount();
		vertexBuffer.elementSize = mesh.GetVertexStride();
		vertexBuffer.flag = static_cast<GPUAsset::Flag>(GPUAsset::SRV | GPUAsset::BUFFER);
		indexBuffer.elementCount = mesh.GetIndices().size();
		indexBuffer.elementSize = sizeof(uint32_t);
		indexBuffer.flag = static_cast<GPUAsset::Flag>(GPUAsset::SRV | GPUAsset::BUFFER);
		if (this->subMeshes)
		{
			for (auto& subMesh : this->subMeshes->subMeshes)
			{
				subMesh.subMeshID = utl::GenerateRandomID();
			}
		}
	}
	std::shared_ptr<Mesh> mesh;
	std::optional<SubMeshes> subMeshes = std::nullopt;
	GPUAsset vertexBuffer;
	GPUAsset indexBuffer;
	bool inludedInAccelerationStructure = false;
};

struct MaterialAsset
{
	MaterialAsset() = default;
	MaterialAsset(const Material& material)
	{
		this->material = std::make_shared<Material>(material);
		constantBuffer.elementCount = 1;
		constantBuffer.elementSize = 2 * sizeof(rfm::Vector4) + sizeof(int);
		constantBuffer.flag = GPUAsset::Flag::CBV;
	}
	std::shared_ptr<Material> material;
	GPUAsset constantBuffer;
	GPUAsset albedoTexture;
};

class AssetManager
{
public:
	static constexpr int maxNumMaterials = 20000;
	static constexpr int maxNumAlbedoTextures = 100;
	static constexpr int maxNumIndexBuffers = 100;
	static constexpr int maxNumVertexBuffers = 100;

	static void Init(Renderer* renderer);
	static void Destroy();
	static AssetManager& Get();
	static bool IsValid();

	uint64_t AddMesh(const Mesh& mesh, bool inludeInAccelerationStructure = true, const std::optional<SubMeshes>& subMeshes = std::nullopt);
	uint64_t AddMaterial(const Material &material);
	uint64_t AddTextureFromFile(const std::string& path, bool mipmapping, bool linearColorSpace);

	void MoveMeshToGPU(uint64_t id);
	void MoveMaterialToGPU(uint64_t id);

	const MeshAsset& GetMesh(uint64_t id) const;
	const MaterialAsset& GetMaterial(uint64_t id) const;
	const GPUAsset& GetTexture(uint64_t id) const;

	//none of the remove functions are tested, they might not work
	void RemoveMesh(uint64_t id);
	void RemoveMaterial(uint64_t id);
	void RemoveTexture(uint64_t id);

	void Update(int numberOfFramesInFlight);

	const std::unordered_map<uint64_t, MeshAsset>& GetAllMeshes() const;

	DescriptorHandle GetBindlessAlbedoTexturesStart() const;
	DescriptorHandle GetBindlessMaterialStart() const;
	DescriptorHandle GetBindlessIndexBufferStart() const;
	DescriptorHandle GetBindlessVertexBufferStart() const;

private:
	AssetManager(Renderer* renderer);
	~AssetManager();
	AssetManager(const AssetManager& other) = delete;
	AssetManager& operator=(const AssetManager& other) = delete;

	static AssetManager* s_instance;

	std::unordered_map<uint64_t, MeshAsset> m_meshes;
	std::unordered_map<uint64_t, MaterialAsset> m_materials;
	std::unordered_map<uint64_t, GPUAsset> m_textures;
	std::queue<std::pair<GPUAsset, uint64_t>> m_gpuAssetsToRemove; //the uint is the frame the remove was requested on
	DescriptorVector m_heapDescriptor = DescriptorVector(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

	DescriptorHandle m_albedoViewsHandle;
	int m_albedoViewCount = 0;
	DescriptorHandle m_materialViewsHandle;
	int m_materialViewCount = 0;
	DescriptorHandle m_vbViewsHandle;
	int m_vbViewCount = 0;
	DescriptorHandle m_ibViewsHandle;
	int m_ibViewCount = 0;

	Renderer* m_renderer = nullptr;
	ID3D12CommandQueue* m_cpyCmdQueue = nullptr;
	ID3D12CommandAllocator* m_cpyCmdAllocator = nullptr;
	ID3D12GraphicsCommandList* m_cpyCmdList = nullptr;

	ID3D12Fence* m_fence = nullptr;
	UINT64 m_fenceValue = 0;

	ID3D12Resource* m_uploadBuffer = nullptr;
	ID3D12Heap* m_uploadHeap = nullptr;
	UINT64 m_uploadHeapSize = 0;
	UINT64 m_uploadBufferOffset = 0;

	struct Image
	{
		std::byte* dataPtr = nullptr;
		int width;
		int height;
		std::string filePath;
	};

	static Image LoadImageFromFile(const std::string& path);
	void RecordUpload();
	void ExecuteUpload();

	void CreateBuffer(GPUAsset& buffer);
	void CreateTexture2D(GPUAsset& texture, uint32_t width, uint32_t height, bool mipmapping, bool linearColorSpace);
	void UploadBufferStaged(const GPUAsset& target, const void* data);
	void UploadTextureStaged(const GPUAsset& target, const void* data);
};