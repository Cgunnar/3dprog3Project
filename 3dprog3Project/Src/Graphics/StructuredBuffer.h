#pragma once
class StructuredBuffer
{
public:
	StructuredBuffer(ID3D12Device* device, UINT elementSize, UINT maxNumElements, bool cpuWrite);
	StructuredBuffer(const StructuredBuffer& other) = delete;
	StructuredBuffer& operator=(const StructuredBuffer& other) = delete;
	~StructuredBuffer();

	void Update(const void* data, UINT count);
private:

	ID3D12Resource* m_buffer = nullptr;
};

