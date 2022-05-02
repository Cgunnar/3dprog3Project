#include "pch.h"
#include "ConstantBufferManager.h"

ConstantBufferManager::ConstantBufferManager(ID3D12Device* device, UINT numConstantBuffers, UINT bufferSize)
	: m_maxNumConstantBuffers(numConstantBuffers), m_bufferSize(utl::AlignSize(bufferSize, 256))
{
	D3D12_RESOURCE_DESC desc;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	desc.MipLevels = 1;
	desc.DepthOrArraySize = 1;
	desc.Height = 1;
	desc.Width = m_bufferSize * numConstantBuffers;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
		&desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(ID3D12Resource),
		reinterpret_cast<void**>(&m_constantBufferPool));
	assert(SUCCEEDED(hr));

	D3D12_RANGE emptyRange = { 0,0 };
	hr = m_constantBufferPool->Map(0, &emptyRange, reinterpret_cast<void**>(&m_beginPtr));
	assert(SUCCEEDED(hr));


	m_heapDescriptors = new DescriptorVector(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, numConstantBuffers);
	m_heapDescriptors->Init(device);

	for (UINT i = 0; i < numConstantBuffers; i++)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc;
		viewDesc.BufferLocation = m_constantBufferPool->GetGPUVirtualAddress() + i * m_bufferSize;
		viewDesc.SizeInBytes = m_bufferSize;
		m_heapDescriptors->CreateConstantBuffer(device, &viewDesc);
	}
}

ConstantBufferManager::~ConstantBufferManager()
{
	delete m_heapDescriptors;
	m_constantBufferPool->Unmap(0, nullptr);
	m_constantBufferPool->Release();
}

UINT ConstantBufferManager::PushConstantBuffer()
{
	assert(m_numUsedConstantBuffers < m_maxNumConstantBuffers);
	return m_numUsedConstantBuffers++;
}

void ConstantBufferManager::PopConstantBuffer()
{
	assert(m_numUsedConstantBuffers > 0);
	--m_numUsedConstantBuffers;
}

void ConstantBufferManager::Clear()
{
	m_numUsedConstantBuffers = 0;
}

void ConstantBufferManager::UpdateConstantBuffer(UINT index, const void* data, UINT size)
{
	assert(index < m_numUsedConstantBuffers && data && size <= m_bufferSize);

	std::memcpy(m_beginPtr + m_bufferSize * index, data, size);
}

D3D12_CPU_DESCRIPTOR_HANDLE ConstantBufferManager::GetCPUHandle(UINT index) const
{
	assert(index < m_numUsedConstantBuffers);
	return m_heapDescriptors->operator[](index);
}

D3D12_GPU_DESCRIPTOR_HANDLE ConstantBufferManager::GetGPUHandle(UINT index) const
{
	assert(index < m_numUsedConstantBuffers);
	return m_heapDescriptors->GetGPUHandle(index);
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBufferManager::GetGPUVirtualAddress(UINT index) const
{
	assert(index < m_numUsedConstantBuffers);
	return m_constantBufferPool->GetGPUVirtualAddress() + m_bufferSize * index;
}

const DescriptorVector& ConstantBufferManager::GetAllDescriptors() const
{
	return *m_heapDescriptors;
}
