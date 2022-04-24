#include "pch.h"
#include "ConstantBufferManager.h"

ConstantBufferManager::ConstantBufferManager(ID3D12Device* device, UINT numConstantBuffers)
	: m_maxNumConstantBuffers(numConstantBuffers)
{
	D3D12_RESOURCE_DESC desc;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	desc.MipLevels = 1;
	desc.DepthOrArraySize = 1;
	desc.Height = 1;
	desc.Width = 256 * numConstantBuffers;
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

	m_currentPtr = m_beginPtr;
	m_endPtr = m_beginPtr + desc.Width;


	m_heapDescriptors = new DescriptorVector(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, numConstantBuffers);
	m_heapDescriptors->Init(device);
}

ConstantBufferManager::~ConstantBufferManager()
{
	delete m_heapDescriptors;
	m_constantBufferPool->Unmap(0, nullptr);
	m_constantBufferPool->Release();
}
