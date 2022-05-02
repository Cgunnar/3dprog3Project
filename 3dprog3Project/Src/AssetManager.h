#pragma once

#include "Mesh.h"
#include "Material.h"
#include "DescriptorVector.h"



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

struct MeshAsset
{
	MeshAsset() = default;
	MeshAsset(const Mesh& mesh)
	{
		this->mesh = std::make_shared<Mesh>(mesh);
		vertexBuffer.elementCount = mesh.GetVertexCount();
		vertexBuffer.elementSize = mesh.GetVertexStride();
		vertexBuffer.flag = static_cast<GPUAsset::Flag>(GPUAsset::SRV | GPUAsset::BUFFER);
		indexBuffer.elementCount = mesh.GetIndices().size();
		indexBuffer.elementSize = sizeof(uint32_t);
		indexBuffer.flag = static_cast<GPUAsset::Flag>(GPUAsset::SRV | GPUAsset::BUFFER);
	}
	std::shared_ptr<Mesh> mesh;
	GPUAsset vertexBuffer;
	GPUAsset indexBuffer;
};

struct MaterialAsset
{
	MaterialAsset() = default;
	MaterialAsset(const Material& material)
	{
		this->material = std::make_shared<Material>(material);
		constantBuffer.elementCount = 1;
		constantBuffer.elementSize = sizeof(rfm::Vector4);
		constantBuffer.flag = GPUAsset::Flag::CBV;
	}
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

	void MoveMeshToGPU(uint64_t id);
	void MoveMaterialToGPU(uint64_t id);

	const MeshAsset& GetMesh(uint64_t id) const;
	const MaterialAsset& GetMaterial(uint64_t id) const;

	void RemoveMesh(uint64_t id);
	void RemoveMaterial(uint64_t id);

	void Update(int numberOfFramesInFlight);

	const DescriptorVector& GetHeapDescriptors() const;

private:
	AssetManager(ID3D12Device* device);
	~AssetManager();
	AssetManager(const AssetManager& other) = delete;
	AssetManager& operator=(const AssetManager& other) = delete;

	static AssetManager* s_instance;

	std::unordered_map<uint64_t, MeshAsset> m_meshes;
	std::unordered_map<uint64_t, MaterialAsset> m_materials;
	std::queue<std::pair<GPUAsset, uint64_t>> m_gpuAssetsToRemove; //the uint is the frame the remove was requested on
	DescriptorVector m_heapDescriptor = DescriptorVector(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

	ID3D12Device* m_device = nullptr;
	ID3D12CommandQueue* m_cpyCmdQueue = nullptr;
	ID3D12CommandAllocator* m_cpyCmdAllocator = nullptr;
	ID3D12GraphicsCommandList* m_cpyCmdList = nullptr;

	ID3D12Fence* m_fence = nullptr;
	UINT64 m_fenceValue = 0;

	ID3D12Resource* m_uploadBuffer = nullptr;
	ID3D12Heap* m_uploadHeap = nullptr;
	UINT64 m_uploadHeapSize = 0;
	UINT64 m_uploadBufferOffset = 0;

	void RecordUpload();
	void ExecuteUpload();

	void CreateBuffer(GPUAsset& buffer);
	void UploadBufferStaged(const GPUAsset& target, const void* data);
};