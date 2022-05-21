#include "pch.h"
#include "AccelerationStructure.h"

#include "rfEntity.hpp"
#include "CommonComponents.h"
#include "AssetManager.h"

AccelerationStructure::AccelerationStructure(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList)
{

	BuildBottomLevel(device, cmdList);
	std::vector<D3D12_RESOURCE_BARRIER> barriers;
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	for (auto& b : m_bottomLevels)
	{
		barrier.UAV.pResource = b.second.resultBuffer;
		barriers.push_back(barrier);
	}
	cmdList->ResourceBarrier(barriers.size(), barriers.data());

	BuildTopLevel(device, cmdList);

	barrier.UAV.pResource = m_topLevel.resultBuffer;
	cmdList->ResourceBarrier(1, &barrier);


	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 1;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	HRESULT hr = device->CreateDescriptorHeap(&descHeapDesc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(&m_descriptorHeap));
	assert(SUCCEEDED(hr));

	D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	viewDesc.Format = DXGI_FORMAT_UNKNOWN;
	viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewDesc.RaytracingAccelerationStructure.Location = m_topLevel.resultBuffer->GetGPUVirtualAddress();

	D3D12_CPU_DESCRIPTOR_HANDLE heapHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	device->CreateShaderResourceView(nullptr, &viewDesc, heapHandle);
}

AccelerationStructure::~AccelerationStructure()
{
	for (auto& bt : m_bottomLevels)
	{
		bt.second.resultBuffer->Release();
		bt.second.scratchBuffer->Release();
	}
	if (m_topLevel.resultBuffer) m_topLevel.resultBuffer->Release();
	if (m_topLevel.scratchBuffer) m_topLevel.scratchBuffer->Release();
	if (m_descriptorHeap) m_descriptorHeap->Release();
}

D3D12_CPU_DESCRIPTOR_HANDLE AccelerationStructure::GetCpuHandle() const
{
	return m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

void AccelerationStructure::UpdateTopLevel(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	ID3D12Device5* device5 = nullptr;
	HRESULT hr = device->QueryInterface(__uuidof(ID3D12Device5), reinterpret_cast<void**>(&device5));
	assert(SUCCEEDED(hr));
	ID3D12GraphicsCommandList4* cmdList4 = nullptr;
	hr = cmdList->QueryInterface(__uuidof(ID3D12GraphicsCommandList4), reinterpret_cast<void**>(&cmdList4));
	assert(SUCCEEDED(hr));

	for (auto& inst : m_instances)
	{
		rfm::Matrix worldMatrix = rfe::EntityReg::GetComponent<TransformComp>(inst.InstanceID)->transform;
		inst.Transform[0][0] = worldMatrix[0][0]; inst.Transform[0][1] = worldMatrix[1][0]; inst.Transform[0][2] = worldMatrix[2][0]; inst.Transform[0][3] = worldMatrix[3][0];
		inst.Transform[1][0] = worldMatrix[0][1]; inst.Transform[1][1] = worldMatrix[1][1]; inst.Transform[1][2] = worldMatrix[2][1]; inst.Transform[1][3] = worldMatrix[3][1];
		inst.Transform[2][0] = worldMatrix[0][2]; inst.Transform[2][1] = worldMatrix[1][2]; inst.Transform[2][2] = worldMatrix[2][2]; inst.Transform[2][3] = worldMatrix[3][2];
	}

	m_instanceBuffer->Update(m_instances.data(), m_instances.size());


	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC accStructureDesc{};
	accStructureDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	accStructureDesc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	accStructureDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	accStructureDesc.Inputs.NumDescs = m_instances.size();
	accStructureDesc.Inputs.InstanceDescs = m_instanceBuffer->GpuAddress();

	accStructureDesc.DestAccelerationStructureData = m_topLevel.resultBuffer->GetGPUVirtualAddress();
	accStructureDesc.ScratchAccelerationStructureData = m_topLevel.scratchBuffer->GetGPUVirtualAddress();
	cmdList4->BuildRaytracingAccelerationStructure(&accStructureDesc, 0, nullptr);

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = m_topLevel.resultBuffer;
	cmdList4->ResourceBarrier(1, &barrier);

	device5->Release();
	cmdList4->Release();
}

bool AccelerationStructure::BuildBottomLevel(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList)
{
	const auto& am = AssetManager::Get();

	const auto& meshes = am.GetAllMeshes();
	
	int count = 0;
	for (const auto& [id, mesh] : meshes)
	{
		auto& botLvl = m_bottomLevels[id];
		D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc;
		geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
		geometryDesc.Triangles.Transform3x4 = NULL;
		geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
		geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geometryDesc.Triangles.IndexCount = mesh.indexBuffer.elementCount;
		geometryDesc.Triangles.VertexCount = mesh.vertexBuffer.elementCount;
		geometryDesc.Triangles.IndexBuffer = mesh.indexBuffer.resource->GetGPUVirtualAddress();
		geometryDesc.Triangles.VertexBuffer.StartAddress = mesh.vertexBuffer.resource->GetGPUVirtualAddress();
		geometryDesc.Triangles.VertexBuffer.StrideInBytes = mesh.vertexBuffer.elementSize;

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC accStructureDesc{};
		accStructureDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		accStructureDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		accStructureDesc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		accStructureDesc.Inputs.NumDescs = 1;
		accStructureDesc.Inputs.pGeometryDescs = &geometryDesc;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo;
		device->GetRaytracingAccelerationStructurePrebuildInfo(&accStructureDesc.Inputs, &prebuildInfo);

		D3D12_RESOURCE_DESC bufferDesc;
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		bufferDesc.MipLevels = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.Height = 1;
		bufferDesc.Width = prebuildInfo.ResultDataMaxSizeInBytes;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		D3D12_HEAP_PROPERTIES heapProps{};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

		HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
			&bufferDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&botLvl.resultBuffer));
		assert(SUCCEEDED(hr));


		bufferDesc.Width = prebuildInfo.ScratchDataSizeInBytes;

		hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
			&bufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&botLvl.scratchBuffer));
		assert(SUCCEEDED(hr));


		accStructureDesc.DestAccelerationStructureData = botLvl.resultBuffer->GetGPUVirtualAddress();
		accStructureDesc.ScratchAccelerationStructureData = botLvl.scratchBuffer->GetGPUVirtualAddress();

		cmdList->BuildRaytracingAccelerationStructure(&accStructureDesc, 0, nullptr);
		count++;
	}
	
	return true;
}

void AccelerationStructure::BuildTopLevel(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList)
{
	auto entitis = rfe::EntityReg::ViewEntities<MeshComp, MaterialComp>();
	UINT numInstances = entitis.size();

	m_instanceBuffer = std::make_unique<StructuredBuffer<D3D12_RAYTRACING_INSTANCE_DESC>>(device, numInstances, false, true);

	const auto& am = AssetManager::Get();
	

	for (auto& e : entitis)
	{
		const MaterialAsset& mat = am.GetMaterial(e.GetComponent<MaterialComp>()->materialID);
		uint64_t meshID = e.GetComponent<MeshComp>()->meshID;

		D3D12_RAYTRACING_INSTANCE_DESC instDesc{};
		instDesc.Transform[0][0] = 1;
		instDesc.Transform[1][1] = 1;
		instDesc.Transform[2][2] = 1;
		instDesc.InstanceID = e;
		assert(am.GetMesh(meshID).indexBuffer.descIndex == am.GetMesh(meshID).vertexBuffer.descIndex);
		//InstanceContributionToHitGroupIndex is not used when using inline raytracing, use it for materialIndex and meshIndex
		//meshIndex is the index of the ib and vb, assumed to have the same index
		UINT meshBuffersIndex = am.GetMesh(meshID).indexBuffer.descIndex;
		assert(mat.constantBuffer.descIndex < 0xffff && meshBuffersIndex < 0xffff);
		instDesc.InstanceContributionToHitGroupIndex = mat.constantBuffer.descIndex | (meshBuffersIndex << 16);
		instDesc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
		instDesc.InstanceMask = 0xFF;
		instDesc.AccelerationStructure = m_bottomLevels[meshID].resultBuffer->GetGPUVirtualAddress();

		m_instances.push_back(instDesc);
	}

	m_instanceBuffer->Update(m_instances.data(), m_instances.size());


	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC accStructureDesc{};
	accStructureDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	accStructureDesc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	accStructureDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	accStructureDesc.Inputs.NumDescs = m_instances.size();
	accStructureDesc.Inputs.InstanceDescs = m_instanceBuffer->GpuAddress();


	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo;
	device->GetRaytracingAccelerationStructurePrebuildInfo(&accStructureDesc.Inputs, &prebuildInfo);

	D3D12_RESOURCE_DESC bufferDesc;
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	bufferDesc.MipLevels = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.Height = 1;
	bufferDesc.Width = prebuildInfo.ResultDataMaxSizeInBytes;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&m_topLevel.resultBuffer));
	assert(SUCCEEDED(hr));


	bufferDesc.Width = prebuildInfo.ScratchDataSizeInBytes;

	hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&m_topLevel.scratchBuffer));
	assert(SUCCEEDED(hr));


	accStructureDesc.DestAccelerationStructureData = m_topLevel.resultBuffer->GetGPUVirtualAddress();
	accStructureDesc.ScratchAccelerationStructureData = m_topLevel.scratchBuffer->GetGPUVirtualAddress();
	cmdList->BuildRaytracingAccelerationStructure(&accStructureDesc, 0, nullptr);
}
