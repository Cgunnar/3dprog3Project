#include "pch.h"
#include "AccelerationStructure.h"

#include "rfEntity.hpp"
#include "CommonComponents.h"
#include "AssetManager.h"

AccelerationStructure::AccelerationStructure(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList)
{

	BuildBottomLevel(device, cmdList);


}

AccelerationStructure::~AccelerationStructure()
{
	if (m_resultBufferBottom) m_resultBufferBottom->Release();
	if (m_scratchBufferBottom) m_scratchBufferBottom->Release();
}

bool AccelerationStructure::BuildBottomLevel(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList)
{
	const auto& am = AssetManager::Get();

	const auto& meshes = am.GetAllMeshes();

	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs;
	geometryDescs.resize(meshes.size());
	int count = 0;
	for (const auto& [id, mesh] : meshes)
	{
		geometryDescs[count].Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geometryDescs[count].Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
		geometryDescs[count].Triangles.Transform3x4 = NULL;
		geometryDescs[count].Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
		geometryDescs[count].Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geometryDescs[count].Triangles.IndexCount = mesh.indexBuffer.elementCount;
		geometryDescs[count].Triangles.VertexCount = mesh.vertexBuffer.elementCount;
		geometryDescs[count].Triangles.IndexBuffer = mesh.indexBuffer.resource->GetGPUVirtualAddress();
		geometryDescs[count].Triangles.VertexBuffer.StartAddress = mesh.vertexBuffer.resource->GetGPUVirtualAddress();
		geometryDescs[count].Triangles.VertexBuffer.StrideInBytes = mesh.vertexBuffer.elementSize;
		count++;
	}

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC accStructureDesc{};
	accStructureDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	accStructureDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	accStructureDesc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	accStructureDesc.Inputs.NumDescs = count;
	accStructureDesc.Inputs.pGeometryDescs = geometryDescs.data();



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
		nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&m_resultBufferBottom));
	assert(SUCCEEDED(hr));


	bufferDesc.Width = prebuildInfo.ScratchDataSizeInBytes;

	hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
		&bufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&m_scratchBufferBottom));
	assert(SUCCEEDED(hr));
	

	accStructureDesc.DestAccelerationStructureData = m_resultBufferBottom->GetGPUVirtualAddress();
	accStructureDesc.ScratchAccelerationStructureData = m_scratchBufferBottom->GetGPUVirtualAddress();

	cmdList->BuildRaytracingAccelerationStructure(&accStructureDesc, 0, nullptr);
	return true;
}
