#pragma once
#include "StructuredBuffer.h"

class AccelerationStructure
{
public:
	AccelerationStructure(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList);
	~AccelerationStructure();


private:

	std::unique_ptr<StructuredBuffer<D3D12_RAYTRACING_INSTANCE_DESC>> m_instanceBuffer;
	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> m_instances;
	std::vector<size_t> m_freeInstanceSlotts;

	struct AccelerationLevel
	{
		ID3D12Resource* resultBuffer = nullptr;
		ID3D12Resource* scratchBuffer = nullptr;
	};
	std::unordered_map<uint64_t, AccelerationLevel> m_bottomLevels;
	AccelerationLevel m_topLevel;

	void BuildTopLevel(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList);
	bool BuildBottomLevel(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList);
	//D3D12_RAYTRACING_INSTANCE_DESC
};

