#include "pch.h"
#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* device, UINT width, UINT height) : m_width(width), m_height(height)
{
	utl::PrintDebug("Render resolution: " + std::to_string(width) + " x " + std::to_string(height));

	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC rtvDesc;
	rtvDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	//rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.MipLevels = 1;
	rtvDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	rtvDesc.Width = width;
	rtvDesc.Height = height;
	rtvDesc.DepthOrArraySize = 1;
	rtvDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rtvDesc.SampleDesc.Count = 1;
	rtvDesc.SampleDesc.Quality = 0;
	rtvDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE rtClarValue;
	rtClarValue.Format = rtvDesc.Format;
	rtClarValue.Color[0] = 0.2f;
	rtClarValue.Color[1] = 0.0f;
	rtClarValue.Color[2] = 0.0f;
	rtClarValue.Color[3] = 0.0f;

	HRESULT hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &rtvDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET, &rtClarValue, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&renderTarget));
	assert(SUCCEEDED(hr));

	if (FAILED(hr)) //troll resolution settings will fail here
	{
		throw std::runtime_error("Failed to create render target");
	}

	D3D12_DESCRIPTOR_HEAP_DESC rtvDescHDesc;
	rtvDescHDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescHDesc.NumDescriptors = 1;
	rtvDescHDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvDescHDesc.NodeMask = 0;

	m_rtvHeapDescIncremenSize = device->GetDescriptorHandleIncrementSize(rtvDescHDesc.Type);
	hr = device->CreateDescriptorHeap(&rtvDescHDesc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(&m_rtvDescHeap));
	assert(SUCCEEDED(hr));

	device->CreateRenderTargetView(renderTarget, nullptr, m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_DESCRIPTOR_HEAP_DESC srvDescHDesc;
	srvDescHDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvDescHDesc.NumDescriptors = 1;
	srvDescHDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	srvDescHDesc.NodeMask = 0;

	hr = device->CreateDescriptorHeap(&srvDescHDesc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(&m_rtAsSrvDescHeap));
	assert(SUCCEEDED(hr));

	device->CreateShaderResourceView(renderTarget, nullptr, m_rtAsSrvDescHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_CLEAR_VALUE depthClarValue;
	depthClarValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthClarValue.DepthStencil.Depth = 1.0f;

	D3D12_RESOURCE_DESC dsvDesc;
	dsvDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	dsvDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	dsvDesc.Width = width;
	dsvDesc.Height = height;
	dsvDesc.MipLevels = 1;
	dsvDesc.DepthOrArraySize = 1;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.SampleDesc.Count = 1;
	dsvDesc.SampleDesc.Quality = 0;
	dsvDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
	dsvDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	hr = device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &dsvDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClarValue, __uuidof(ID3D12Resource),
		reinterpret_cast<void**>(&depthBuffer));
	assert(SUCCEEDED(hr));

	D3D12_DESCRIPTOR_HEAP_DESC dsvDescHDesc;
	dsvDescHDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDescHDesc.NumDescriptors = 1;
	dsvDescHDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvDescHDesc.NodeMask = 0;

	hr = device->CreateDescriptorHeap(&dsvDescHDesc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(&m_dsvDescHeap));
	assert(SUCCEEDED(hr));

	device->CreateDepthStencilView(depthBuffer, nullptr, m_dsvDescHeap->GetCPUDescriptorHandleForHeapStart());
}

FrameResource::~FrameResource()
{
	if (m_dsvDescHeap) m_dsvDescHeap->Release();
	if (m_rtvDescHeap) m_rtvDescHeap->Release();
	if (m_rtAsSrvDescHeap) m_rtAsSrvDescHeap->Release();
	if (renderTarget) renderTarget->Release();
	if (depthBuffer) depthBuffer->Release();
}

FrameResource::FrameResource(FrameResource&& other) noexcept
{
	m_width = other.m_width;
	m_height = other.m_height;
	depthBuffer = other.depthBuffer;
	renderTarget = other.renderTarget;
	m_dsvDescHeap = other.m_dsvDescHeap;
	m_rtvDescHeap = other.m_rtvDescHeap;
	m_rtAsSrvDescHeap = other.m_rtAsSrvDescHeap;
	m_rtvHeapDescIncremenSize = other.m_rtvHeapDescIncremenSize;
	m_backBufferCpuDescHandle = other.m_backBufferCpuDescHandle;

	other.depthBuffer = nullptr;
	other.renderTarget = nullptr;
	other.m_dsvDescHeap = nullptr;
	other.m_rtvDescHeap = nullptr;
	other.m_rtAsSrvDescHeap = nullptr;
}

FrameResource& FrameResource::operator=(FrameResource&& other) noexcept
{
	if (m_dsvDescHeap) m_dsvDescHeap->Release();
	if (m_rtvDescHeap) m_rtvDescHeap->Release();
	if (m_rtAsSrvDescHeap) m_rtAsSrvDescHeap->Release();
	if (renderTarget) renderTarget->Release();
	if (depthBuffer) depthBuffer->Release();

	m_width = other.m_width;
	m_height = other.m_height;
	depthBuffer = other.depthBuffer;
	renderTarget = other.renderTarget;
	m_dsvDescHeap = other.m_dsvDescHeap;
	m_rtvDescHeap = other.m_rtvDescHeap;
	m_rtAsSrvDescHeap = other.m_rtAsSrvDescHeap;
	m_rtvHeapDescIncremenSize = other.m_rtvHeapDescIncremenSize;
	m_backBufferCpuDescHandle = other.m_backBufferCpuDescHandle;

	other.depthBuffer = nullptr;
	other.renderTarget = nullptr;
	other.m_dsvDescHeap = nullptr;
	other.m_rtvDescHeap = nullptr;
	other.m_rtAsSrvDescHeap = nullptr;

	return *this;
}

std::pair<UINT, UINT> FrameResource::GetResolution() const
{
	return std::make_pair(m_width, m_height);
}

D3D12_CPU_DESCRIPTOR_HANDLE FrameResource::GetDsvCpuHandle() const
{
	return m_dsvDescHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE FrameResource::GetRtvCpuHandle() const
{
	return m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE FrameResource::GetRtSrvCpuHandle() const
{
	return m_rtAsSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE FrameResource::GetBackBufferCpuHandle() const
{
	return m_backBufferCpuDescHandle;
}
