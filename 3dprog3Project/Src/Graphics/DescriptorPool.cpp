#include "pch.h"
#include "DescriptorPool.h"

DescriptorPool::DescriptorPool(ID3D12Device* device, UINT framesInFlight, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT dynamicMaxSize, UINT staticMaxSize)
{
	m_device = device;
	m_staticBegin = dynamicMaxSize;
	m_staticEnd = dynamicMaxSize + staticMaxSize;
	m_framesInFlight = framesInFlight;
	m_perFramePoolSize = dynamicMaxSize / m_framesInFlight;
	m_descriptorHeapDesc.Flags = flags;
	m_descriptorHeapDesc.NodeMask = 0;
	m_descriptorHeapDesc.Type = type;
	m_descriptorHeapDesc.NumDescriptors = dynamicMaxSize + staticMaxSize;

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
	m_dynamicCount = 0;
}

DescriptorHandle DescriptorPool::DynamicAllocate(UINT count)
{
	assert(m_dynamicCount + count < m_perFramePoolSize);
	if (!(m_dynamicCount + count < m_perFramePoolSize))
	{
		throw std::runtime_error("out of memory in descriptor pool");
	}
	DescriptorHandle handle;
	handle.gpuHandle = m_descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handle.gpuHandle.ptr += (m_frameIndex * m_perFramePoolSize + m_dynamicCount) * m_incrementSize;

	handle.cpuHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handle.cpuHandle.ptr += (m_frameIndex * m_perFramePoolSize + m_dynamicCount) * m_incrementSize;

	handle.incrementSize = m_incrementSize;
	handle.count = count;
	handle.index = m_frameIndex * m_perFramePoolSize + m_dynamicCount;
	m_dynamicCount += count;
	return handle;
}

DescriptorHandle DescriptorPool::StaticAllocate(UINT count)
{
	assert(m_staticBegin + m_staticCount + count < m_staticEnd);
	if (!(m_staticBegin + m_staticCount + count < m_staticEnd))
	{
		std::runtime_error("out of memory in descriptor pool");
	}
	DescriptorHandle handle;
	handle.gpuHandle = m_descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handle.gpuHandle.ptr += (m_staticBegin + m_staticCount) * m_incrementSize;

	handle.cpuHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handle.cpuHandle.ptr += (m_staticBegin + m_staticCount) * m_incrementSize;

	handle.incrementSize = m_incrementSize;
	handle.count = count;
	handle.index = m_staticBegin + m_staticCount;
	m_staticCount += count;
	return handle;
}

ID3D12DescriptorHeap* DescriptorPool::Get() const
{
	return m_descriptorHeap;
}
