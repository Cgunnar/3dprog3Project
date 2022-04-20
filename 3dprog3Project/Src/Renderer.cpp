#include "pch.h"
#include "Renderer.h"

#pragma comment( lib, "d3d12.lib")
#pragma comment( lib, "d3dcompiler.lib")
#pragma comment( lib, "dxgi.lib")

Renderer::Renderer(HWND windowHandle)
{
	IDXGIFactory6* factory6;
	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory6), reinterpret_cast<void**>(&factory6));
	assert(SUCCEEDED(hr));

	CreateDeviceAndDirectCmd(factory6);
	CreateSwapChain(factory6, windowHandle);

	factory6->Release();



	D3D12_DESCRIPTOR_HEAP_DESC rtvDescHDesc
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = static_cast<UINT>(m_backbuffers.size()),
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		.NodeMask = 0u
	};

	hr = m_device->CreateDescriptorHeap(&rtvDescHDesc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(&m_rtvDescHeap));
	assert(SUCCEEDED(hr));

	D3D12_CPU_DESCRIPTOR_HANDLE heapHandle;

	m_rtvDescSize = m_device->GetDescriptorHandleIncrementSize(rtvDescHDesc.Type);
	heapHandle = m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_backbuffers.size(); i++)
	{
		hr = m_swapchain->GetBuffer(i, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&m_backbuffers[i]));
		assert(SUCCEEDED(hr));
		m_device->CreateRenderTargetView(m_backbuffers[i], nullptr, heapHandle);
		heapHandle.ptr += m_rtvDescSize;
	}



	auto backBufferDesc = m_backbuffers.front()->GetDesc();

	D3D12_CLEAR_VALUE depthClarValue;
	depthClarValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthClarValue.DepthStencil.Depth = 1.0f;
	D3D12_HEAP_PROPERTIES heapZProp = {};
	heapZProp.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC zResoDesc;
	zResoDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	zResoDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	zResoDesc.Width = backBufferDesc.Width;
	zResoDesc.Height = backBufferDesc.Height;
	zResoDesc.MipLevels = 1;
	zResoDesc.DepthOrArraySize = 1;
	zResoDesc.Format = DXGI_FORMAT_D32_FLOAT;
	zResoDesc.SampleDesc.Count = 1;
	zResoDesc.SampleDesc.Quality = 0;
	zResoDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
	zResoDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	hr = m_device->CreateCommittedResource(&heapZProp, D3D12_HEAP_FLAG_NONE, &zResoDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClarValue, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&m_depthBufferResource));
	assert(SUCCEEDED(hr));

	D3D12_DESCRIPTOR_HEAP_DESC dsvDescHDesc
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		.NumDescriptors = 1u,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		.NodeMask = 0u
	};


	hr = m_device->CreateDescriptorHeap(&dsvDescHDesc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(&m_dsvDescHeap));
	assert(SUCCEEDED(hr));

	heapHandle = m_dsvDescHeap->GetCPUDescriptorHandleForHeapStart();
	m_device->CreateDepthStencilView(m_depthBufferResource, nullptr, heapHandle);

}

Renderer::~Renderer()
{
	m_dsvDescHeap->Release();
	m_depthBufferResource->Release();
	m_rtvDescHeap->Release();
	for (auto& bb : m_backbuffers)
		bb->Release();
	m_swapchain->Release();
	m_fence->Release();
	m_directCmdList->Release();
	m_directCmdAllocator->Release();
	m_directCmdQueue->Release();
	m_device->Release();
	m_adapter->Release();
}

void Renderer::BeginFrame()
{
	HRESULT hr = m_directCmdAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = m_directCmdList->Reset(m_directCmdAllocator, nullptr);
	assert(SUCCEEDED(hr));

	m_currentBackbufferIndex = m_swapchain->GetCurrentBackBufferIndex();


	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvDescHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += m_rtvDescSize * m_currentBackbufferIndex;

	D3D12_RESOURCE_BARRIER backbufferTransitionBarrier;
	backbufferTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	backbufferTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	backbufferTransitionBarrier.Transition.pResource = m_backbuffers[m_currentBackbufferIndex];
	backbufferTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	backbufferTransitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	backbufferTransitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	m_directCmdList->ResourceBarrier(1, &backbufferTransitionBarrier);

	float clearColor[] = { 0.2f, 0.0f, 0.0f, 0.0f };
	m_directCmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_directCmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	m_directCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_directCmdList->OMSetRenderTargets(1, &rtvHandle, true, &dsvHandle);


	const auto& backbufferDesc = m_backbuffers.front()->GetDesc();
	UINT width = backbufferDesc.Width;
	UINT height = backbufferDesc.Height;

	D3D12_VIEWPORT viewport = { 0, 0, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
	m_directCmdList->RSSetViewports(1, &viewport);
	D3D12_RECT scissorRect = { 0, 0, static_cast<long>(width), static_cast<long>(height) };
	m_directCmdList->RSSetScissorRects(1, &scissorRect);
}

size_t Renderer::Render()
{
	BeginFrame();



	EndFrame();

	return m_fenceValue;
}



void Renderer::EndFrame()
{
	D3D12_RESOURCE_BARRIER backbufferTransitionBarrier;
	backbufferTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	backbufferTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	backbufferTransitionBarrier.Transition.pResource = m_backbuffers[m_currentBackbufferIndex];
	backbufferTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	backbufferTransitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	backbufferTransitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	m_directCmdList->ResourceBarrier(1, &backbufferTransitionBarrier);

	HRESULT hr = m_directCmdList->Close();
	assert(SUCCEEDED(hr));
	ID3D12CommandList* tmpCmdList = m_directCmdList;
	m_directCmdQueue->ExecuteCommandLists(1, &tmpCmdList);
	m_swapchain->Present(0, 0);
	FrameFence();
}

void Renderer::CreateDeviceAndDirectCmd(IDXGIFactory6* factory)
{
	HRESULT hr = S_OK;
#ifdef _DEBUG
	ID3D12Debug1* dx12debug;
	hr = D3D12GetDebugInterface(__uuidof(ID3D12Debug1), reinterpret_cast<void**>(&dx12debug));
	assert(SUCCEEDED(hr));
	dx12debug->EnableDebugLayer();
	dx12debug->SetEnableGPUBasedValidation(true);
	dx12debug->Release();
#endif // _DEBUG

	hr = factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE::DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
		__uuidof(IDXGIAdapter4), reinterpret_cast<void**>(&m_adapter));
	assert(SUCCEEDED(hr));

	DXGI_ADAPTER_DESC3 adapterDesc;
	m_adapter->GetDesc3(&adapterDesc);
	m_vramInBytes = GetVramInfo(m_adapter).adapterMemory;
	
	utl::PrintDebug(L"Adapter: " + std::wstring(adapterDesc.Description));
	utl::PrintDebug(std::to_string(m_vramInBytes / 1000000) + " MB");

	hr = D3D12CreateDevice(m_adapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), reinterpret_cast<void**>(&m_device));
	assert(SUCCEEDED(hr));


	D3D12_COMMAND_QUEUE_DESC desc;
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = 0;
	desc.NodeMask = 0u;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	hr = m_device->CreateCommandQueue(&desc, __uuidof(ID3D12CommandQueue), reinterpret_cast<void**>(&m_directCmdQueue));
	assert(SUCCEEDED(hr));

	hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), reinterpret_cast<void**>(&m_directCmdAllocator));
	assert(SUCCEEDED(hr));

	hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_directCmdAllocator, nullptr, __uuidof(ID3D12GraphicsCommandList), reinterpret_cast<void**>(&m_directCmdList));
	assert(SUCCEEDED(hr));
	m_directCmdList->Close();

	hr = m_directCmdQueue->SetName(L"mainCmdQueue");
	assert(SUCCEEDED(hr));
	hr = m_directCmdAllocator->SetName(L"mainCmdAllocator");
	assert(SUCCEEDED(hr));
	hr = m_directCmdList->SetName(L"mainCmdList");
	assert(SUCCEEDED(hr));

	hr = m_device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), reinterpret_cast<void**>(&m_fence));
	assert(SUCCEEDED(hr));
}

void Renderer::CreateSwapChain(IDXGIFactory2* factory, HWND windowHandle)
{
	DXGI_SWAP_CHAIN_DESC1 desc;
	desc.Width = 0;
	desc.Height = 0;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Stereo = false;
	desc.SampleDesc = { 1u, 0u };
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = m_backbuffers.size();
	desc.Scaling = DXGI_SCALING_STRETCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hr = factory->CreateSwapChainForHwnd(m_directCmdQueue, windowHandle, &desc, nullptr,
		nullptr, reinterpret_cast<IDXGISwapChain1**>(&m_swapchain));
	assert(SUCCEEDED(hr));
}

MemoryInfo Renderer::GetVramInfo(IDXGIAdapter4* adapter)
{
	DXGI_QUERY_VIDEO_MEMORY_INFO dxgimemInfo;
	HRESULT hr = adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP::DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &dxgimemInfo);
	assert(SUCCEEDED(hr));

	DXGI_ADAPTER_DESC3 desc;
	hr = adapter->GetDesc3(&desc);
	assert(SUCCEEDED(hr));

	MemoryInfo memInfo;
	memInfo.adapterMemory = desc.DedicatedVideoMemory;
	memInfo.memoryBudgetFromOS = dxgimemInfo.Budget;
	memInfo.applicationMemoryUsage = dxgimemInfo.CurrentUsage;

	return memInfo;
}

void Renderer::FrameFence()
{
	m_fenceValue++;
	HRESULT hr = m_directCmdQueue->Signal(m_fence, m_fenceValue);
	assert(SUCCEEDED(hr));

	if (m_fence->GetCompletedValue() < m_fenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, 0, 0, EVENT_ALL_ACCESS);
		hr = m_fence->SetEventOnCompletion(m_fenceValue, eventHandle);
		assert(SUCCEEDED(hr));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}


