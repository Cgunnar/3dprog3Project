#pragma once
class AccelerationStructure
{
public:
	AccelerationStructure(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList);
	~AccelerationStructure();


private:

	bool BuildBottomLevel(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList);

	ID3D12Resource* m_resultBufferBottom = nullptr;
	ID3D12Resource* m_scratchBufferBottom= nullptr;

	//D3D12_RAYTRACING_INSTANCE_DESC
};

