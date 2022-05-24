#pragma once
#include "StructuredBuffer.h"

class AccelerationStructure
{
public:
	AccelerationStructure() = default;
	AccelerationStructure(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList);
	~AccelerationStructure();

	D3D12_CPU_DESCRIPTOR_HANDLE GetAccelerationStructureCpuHandle() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetInstanceMetaDataCpuHandle() const;
	bool UpdateTopLevel(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);

private:
	bool m_valid = false;
	struct TopLevelInstanceMetaData
	{
		int32_t materialDescriptorIndex;
		uint32_t indexBufferDescriptorIndex;
		uint32_t vertexBufferDescriptorIndex;
		uint32_t indexStart;
		uint32_t vertexStart;
	};

	std::unique_ptr<StructuredBuffer<TopLevelInstanceMetaData>> m_instanceMetaDataBuffer;
	std::vector<TopLevelInstanceMetaData> m_instanceMetaData;
	std::unique_ptr<StructuredBuffer<D3D12_RAYTRACING_INSTANCE_DESC>> m_instanceBuffer;
	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> m_instances;
	std::vector<size_t> m_freeInstanceSlotts;

	
	struct AccelerationLevel
	{
		ID3D12Resource* resultBuffer = nullptr;
		ID3D12Resource* scratchBuffer = nullptr;
	};
	std::unordered_map<uint64_t, std::unordered_map<uint64_t, AccelerationLevel>> m_bottomLevels;
	std::unordered_map<uint64_t, size_t> m_instanceToEntityMap;
	AccelerationLevel m_topLevel;
	ID3D12DescriptorHeap* m_descriptorHeap = nullptr;

	void BuildTopLevel(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList);
	bool BuildBottomLevel(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList);
	//D3D12_RAYTRACING_INSTANCE_DESC
};

