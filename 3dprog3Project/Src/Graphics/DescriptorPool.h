#pragma once

struct DescriptorHandle
{
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	UINT incrementSize;
	UINT count;
};

class DescriptorPool
{
public:
	DescriptorPool(ID3D12Device* device, UINT framesInFlight, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT maxSize = 2048 * 8);
	~DescriptorPool();

	DescriptorPool(const DescriptorPool& other) = delete;
	DescriptorPool& operator=(const DescriptorPool& other) = delete;

	void SetNextFrame();
	DescriptorHandle DynamicAllocate(UINT count);

private:
	ID3D12Device* m_device;
	UINT m_count = 0;
	UINT m_perFramePoolSize = 0;
	UINT m_framesInFlight = 0;
	UINT m_capacity = 0;
	UINT m_incrementSize = 0;
	int m_frameIndex = 0;
	ID3D12DescriptorHeap* m_descriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC m_descriptorHeapDesc;

};

