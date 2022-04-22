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

void AssetManager::MoveMeshToGPU(uint64_t id)
{
	assert(m_meshes.contains(id));
	GPUAsset& vb = m_meshes[id].vertexBuffer;

	D3D12_RESOURCE_DESC desc;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	desc.MipLevels = 1;
	desc.DepthOrArraySize = 1;
	desc.Height = 1;
	desc.Width = static_cast<UINT64>(elementSize) * nrOfElements;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
}

void AssetManager::MoveMaterialToGPU(uint64_t id)
{

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
	//the heapdescriptors are still in memory that is not reused in any way

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


	HRESULT hr = device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), reinterpret_cast<void**>(&m_fence));
	assert(SUCCEEDED(hr));

	D3D12_COMMAND_QUEUE_DESC desc;
	desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	desc.Priority = 0;
	desc.NodeMask = 0u;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	hr = device->CreateCommandQueue(&desc, __uuidof(ID3D12CommandQueue), reinterpret_cast<void**>(&m_cpyCmdQueue));
	assert(SUCCEEDED(hr));

	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, __uuidof(ID3D12CommandAllocator), reinterpret_cast<void**>(&m_cpyCmdAllocator));
	assert(SUCCEEDED(hr));

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_cpyCmdAllocator, nullptr, __uuidof(ID3D12GraphicsCommandList), reinterpret_cast<void**>(&m_cpyCmdList));
	assert(SUCCEEDED(hr));

	hr = m_cpyCmdList->Close();
	assert(SUCCEEDED(hr));

	hr = m_cpyCmdQueue->SetName(L"assetManagerCmdQueue");
	assert(SUCCEEDED(hr));
	hr = m_cpyCmdAllocator->SetName(L"assetManagerCmdAllocator");
	assert(SUCCEEDED(hr));
	hr = m_cpyCmdList->SetName(L"assetManagerCmdList");
	assert(SUCCEEDED(hr));


	m_uploadHeapSize = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT * 100; //we will se if this is big enught

	D3D12_HEAP_PROPERTIES uploadHeapProp;
	uploadHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	uploadHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	uploadHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	uploadHeapProp.CreationNodeMask = 0;
	uploadHeapProp.VisibleNodeMask = 0;

	D3D12_HEAP_DESC uploadHeapDesc;
	uploadHeapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	uploadHeapDesc.SizeInBytes = m_uploadHeapSize;
	uploadHeapDesc.Properties = uploadHeapProp;
	uploadHeapDesc.Flags = D3D12_HEAP_FLAG_NONE;

	hr = device->CreateHeap(&uploadHeapDesc, __uuidof(ID3D12Heap), reinterpret_cast<void**>(&m_uploadHeap));
	assert(SUCCEEDED(hr));

	D3D12_RESOURCE_DESC uploadBufferDesc;
	uploadBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	uploadBufferDesc.Alignment = uploadHeapDesc.Alignment;
	uploadBufferDesc.Width = uploadHeapDesc.SizeInBytes;
	uploadBufferDesc.Height = 1;
	uploadBufferDesc.DepthOrArraySize = 1;
	uploadBufferDesc.MipLevels = 1;
	uploadBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	uploadBufferDesc.SampleDesc = { 1u, 0u };
	uploadBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	uploadBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	hr = device->CreatePlacedResource(m_uploadHeap, 0u, &uploadBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&m_uploadBuffer));
	assert(SUCCEEDED(hr));

}

AssetManager::~AssetManager()
{
	while (!m_gpuAssetsToRemove.empty())
	{
		if (m_gpuAssetsToRemove.front().first.resource)
			m_gpuAssetsToRemove.front().first.resource->Release();
		m_gpuAssetsToRemove.pop();
	}


	m_uploadBuffer->Release();
	m_uploadHeap->Release();
	m_cpyCmdAllocator->Release();
	m_cpyCmdQueue->Release();
	m_cpyCmdList->Release();
	m_fence->Release();
}

void AssetManager::RecordUpload()
{
	HRESULT hr = m_cpyCmdAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = m_cpyCmdList->Reset(m_cpyCmdAllocator, nullptr);
	assert(SUCCEEDED(hr));
}

void AssetManager::ExecuteUpload()
{
	HRESULT hr = m_cpyCmdList->Close();
	assert(SUCCEEDED(hr));
	m_cpyCmdQueue->ExecuteCommandLists(1u, reinterpret_cast<ID3D12CommandList**>(&m_cpyCmdList));
	m_fenceValue++;
	hr = m_cpyCmdQueue->Signal(m_fence, m_fenceValue);
	assert(SUCCEEDED(hr));

	if (m_fence->GetCompletedValue() < m_fenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, 0, 0, EVENT_ALL_ACCESS);
		hr = m_fence->SetEventOnCompletion(m_fenceValue, eventHandle);
		assert(SUCCEEDED(hr));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
	m_uploadBufferOffset = 0;
}
