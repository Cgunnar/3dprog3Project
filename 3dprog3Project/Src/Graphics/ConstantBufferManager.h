#pragma once

#include "DescriptorVector.h"
class ConstantBufferManager
{
public:
	ConstantBufferManager(ID3D12Device* device, UINT numConstantBuffers, UINT bufferSize);
	~ConstantBufferManager();

	UINT PushConstantBuffer();
	void PopConstantBuffer();
	void Clear();
	void UpdateConstantBuffer(UINT index, const void* data, UINT size);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UINT index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(UINT index) const;
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress(UINT index) const;
	const DescriptorVector& GetAllDescriptors() const;

private:
	ID3D12Resource* m_constantBufferPool = nullptr;
	std::byte* m_beginPtr = nullptr;
	UINT m_maxNumConstantBuffers = 0;
	UINT m_numUsedConstantBuffers = 0;
	UINT m_bufferSize = 256;
	DescriptorVector* m_heapDescriptors = nullptr;
};

