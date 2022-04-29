#include "pch.h"
#include "DescriptorPool.h"

DescriptorPool::DescriptorPool(ID3D12Device* device, UINT framesInFlight, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT maxSize)
{
	m_device = device;
	m_capacity = maxSize;
	m_framesInFlight = framesInFlight;
	m_perFramePoolSize = maxSize / m_framesInFlight;
	m_descriptorHeapDesc.Flags = flags;
	m_descriptorHeapDesc.NodeMask = 0;
	m_descriptorHeapDesc.Type = type;
	m_descriptorHeapDesc.NumDescriptors = m_capacity;

	m_incrementSize = device->GetDescriptorHandleIncrementSize(type);
	HRESULT hr = device->CreateDescriptorHeap(&m_descriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(&m_descriptorHeap));
	assert(SUCCEEDED(hr));
}

DescriptorPool::~DescriptorPool()
{
	m_descriptorHeap->Release();
}

void DescriptorPool::SetNextFrame()
{
	m_frameIndex++;
	m_frameIndex = m_frameIndex % m_framesInFlight;
	m_count = 0;
}

DescriptorHandle DescriptorPool::DynamicAllocate(UINT count)
{
	assert(m_count + count < m_perFramePoolSize);
	DescriptorHandle handle;
	handle.gpuHandle = m_descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handle.gpuHandle.ptr += (m_frameIndex * m_perFramePoolSize + m_count) * m_incrementSize;

	handle.cpuHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handle.cpuHandle.ptr += (m_frameIndex * m_perFramePoolSize + m_count) * m_incrementSize;

	handle.incrementSize = m_incrementSize;
	handle.count = count;

	m_count += count;
	return handle;
}

ID3D12DescriptorHeap* DescriptorPool::Get() const
{
	return m_descriptorHeap;
}
