#include "pch.h"
#include "StructuredBuffer.h"

StructuredBuffer::StructuredBuffer(ID3D12Device* device, UINT elementSize, UINT maxNumElements, bool cpuWrite)
{
	D3D12_RESOURCE_DESC desc;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	desc.MipLevels = 1;
	desc.DepthOrArraySize = 1;
	desc.Height = 1;
	desc.Width = static_cast<UINT64>(elementSize) * maxNumElements;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	if (cpuWrite)
	{
		D3D12_HEAP_PROPERTIES heapProps{};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
			&desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&m_buffer));
		assert(SUCCEEDED(hr));
	}
	else
	{
		D3D12_HEAP_PROPERTIES heapProps{};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
			&desc, D3D12_RESOURCE_STATE_COMMON, nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&m_buffer));
		assert(SUCCEEDED(hr));
	}
}

StructuredBuffer::~StructuredBuffer()
{
	m_buffer->Release();
}

void StructuredBuffer::Update(const void* data, UINT count)
{

}