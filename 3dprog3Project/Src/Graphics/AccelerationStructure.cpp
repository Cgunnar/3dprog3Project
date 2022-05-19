#include "pch.h"
#include "AccelerationStructure.h"

#include "rfEntity.hpp"
#include "CommonComponents.h"
#include "AssetManager.h"

AccelerationStructure::AccelerationStructure(ID3D12Device5* device)
{

	BuildBottomLevel(device);


}

AccelerationStructure::~AccelerationStructure()
{

}

bool AccelerationStructure::BuildBottomLevel(ID3D12Device5* device)
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

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	inputs.NumDescs = count;
	inputs.pGeometryDescs = geometryDescs.data();
	return true;
}
