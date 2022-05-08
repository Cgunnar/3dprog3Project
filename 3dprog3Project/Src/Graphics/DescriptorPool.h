#pragma once

struct DescriptorHandle
{
	DescriptorHandle operator[](int index) const
	{
		DescriptorHandle handle = *this;
		handle.cpuHandle.ptr += index * incrementSize;
		handle.gpuHandle.ptr += index * incrementSize;
		handle.index += index;
		return handle;
	}
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	UINT incrementSize;
	UINT count;
	UINT index;
};

class DescriptorPool
{
public:
	DescriptorPool(ID3D12Device* device, UINT framesInFlight, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT dynamicMaxSize = 2048 * 8, UINT staticMaxSize = 2048 * 8);
	~DescriptorPool();

	DescriptorPool(const DescriptorPool& other) = delete;
	DescriptorPool& operator=(const DescriptorPool& other) = delete;

	void SetNextFrame();
	DescriptorHandle DynamicAllocate(UINT count);
	DescriptorHandle StaticAllocate(UINT count);
	ID3D12DescriptorHeap* Get() const;

private:
	ID3D12Device* m_device;
	UINT m_dynamicCount = 0;
	UINT m_staticCount = 0;
	UINT m_staticEnd = 0;
	UINT m_perFramePoolSize = 0;
	UINT m_framesInFlight = 0;
	UINT m_staticBegin = 0;
	UINT m_incrementSize = 0;
	int m_frameIndex = 0;
	ID3D12DescriptorHeap* m_descriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC m_descriptorHeapDesc;

};

