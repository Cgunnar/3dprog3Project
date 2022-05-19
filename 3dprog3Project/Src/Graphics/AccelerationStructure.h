#pragma once
class AccelerationStructure
{
public:
	AccelerationStructure(ID3D12Device5* device);
	~AccelerationStructure();


private:

	bool BuildBottomLevel(ID3D12Device5* device);
	//D3D12_RAYTRACING_INSTANCE_DESC
};

