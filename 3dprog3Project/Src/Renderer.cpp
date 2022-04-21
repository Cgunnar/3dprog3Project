#include "pch.h"
#include "Renderer.h"
#include <imgui_impl_dx12.h>

#pragma comment( lib, "d3d12.lib")
#pragma comment( lib, "d3dcompiler.lib")
#pragma comment( lib, "dxgi.lib")

Renderer::Renderer(HWND windowHandle)
{
	HRESULT hr = S_OK;
	UINT factoryFlag = 0;
#ifdef _DEBUG
	ID3D12Debug1* dx12debug;
	hr = D3D12GetDebugInterface(__uuidof(ID3D12Debug1), reinterpret_cast<void**>(&dx12debug));
	assert(SUCCEEDED(hr));
	dx12debug->EnableDebugLayer();
	dx12debug->SetEnableGPUBasedValidation(true);
	dx12debug->Release();

	factoryFlag = DXGI_CREATE_FACTORY_DEBUG;
#endif // _DEBUG

	IDXGIFactory6* factory6;
	hr = CreateDXGIFactory2(factoryFlag, __uuidof(IDXGIFactory6), reinterpret_cast<void**>(&factory6));
	assert(SUCCEEDED(hr));

	CreateDeviceAndDirectCmd(factory6);
	CreateSwapChain(factory6, windowHandle);

	m_fenceValues.resize(m_numFramesInFlight, 0);
	hr = m_device->CreateFence(m_fenceValues[m_currentBackbufferIndex % m_numFramesInFlight], D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), reinterpret_cast<void**>(&m_fence));
	assert(SUCCEEDED(hr));
	m_fenceValues[m_currentBackbufferIndex % m_numFramesInFlight]++;

	m_eventHandle = CreateEventEx(nullptr, 0, 0, EVENT_ALL_ACCESS);
	assert(m_eventHandle);

	factory6->Release();

	D3D12_DESCRIPTOR_HEAP_DESC rtvDescHDesc;
	rtvDescHDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescHDesc.NumDescriptors = static_cast<UINT>(m_backbuffers.size());
	rtvDescHDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvDescHDesc.NodeMask = 0;

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


	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hr = m_device->CreateDescriptorHeap(&desc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(&m_imguiDescHeap));
		assert(SUCCEEDED(hr));

	ImGui_ImplDX12_Init(m_device, m_numFramesInFlight,
		DXGI_FORMAT_R8G8B8A8_UNORM, m_imguiDescHeap,
		m_imguiDescHeap->GetCPUDescriptorHandleForHeapStart(),
		m_imguiDescHeap->GetGPUDescriptorHandleForHeapStart());


	FlushGPU();
}

Renderer::~Renderer()
{
	FlushGPU();

	ImGui_ImplDX12_Shutdown();
	m_imguiDescHeap->Release();

	m_dsvDescHeap->Release();
	m_depthBufferResource->Release();
	m_rtvDescHeap->Release();
	for (auto& bb : m_backbuffers)
		bb->Release();
	m_swapchain->Release();
	m_fence->Release();
	m_directCmdList->Release();
	for (auto& dca : m_directCmdAllocator)
		dca->Release();
	m_directCmdQueue->Release();
	m_device->Release();
	m_adapter->Release();

	CloseHandle(m_eventHandle);
}

void Renderer::BeginFrame()
{
	size_t frameIndex = m_currentBackbufferIndex % m_numFramesInFlight;
	HRESULT hr = m_directCmdAllocator[frameIndex]->Reset();
	assert(SUCCEEDED(hr));
	hr = m_directCmdList->Reset(m_directCmdAllocator[frameIndex], nullptr);
	assert(SUCCEEDED(hr));


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

	m_directCmdList->SetDescriptorHeaps(1, &m_imguiDescHeap);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_directCmdList);
	EndFrame();
	static size_t counter = 0;
	return counter++;
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

	// this comment is from microsofts smmple repo
	// When using sync interval 0, it is recommended to always pass the tearing
	// flag when it is supported, even when presenting in windowed mode.
	// However, this flag cannot be used if the app is in fullscreen mode as a
	// result of calling SetFullscreenState.
	hr = m_swapchain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
	assert(SUCCEEDED(hr));

	FrameFence();
}

void Renderer::CreateDeviceAndDirectCmd(IDXGIFactory6* factory)
{
	HRESULT hr = factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE::DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
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

	m_directCmdAllocator.resize(m_numFramesInFlight, nullptr);
	for (int i = 0; i < m_directCmdAllocator.size(); i++)
	{
		hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), reinterpret_cast<void**>(&m_directCmdAllocator[i]));
		assert(SUCCEEDED(hr));
	}

	hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_directCmdAllocator.front(), nullptr, __uuidof(ID3D12GraphicsCommandList), reinterpret_cast<void**>(&m_directCmdList));
	assert(SUCCEEDED(hr));
	m_directCmdList->Close();

	hr = m_directCmdQueue->SetName(L"mainCmdQueue");
	assert(SUCCEEDED(hr));
	for (int i = 0; i < m_directCmdAllocator.size(); i++)
	{
		std::wstring name = std::wstring(L"mainCmdAllocator: ") + std::to_wstring(i);
		hr = m_directCmdAllocator[i]->SetName(name.c_str());
		assert(SUCCEEDED(hr));
	}
	hr = m_directCmdList->SetName(L"mainCmdList");
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

	hr = factory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER);
	assert(SUCCEEDED(hr));

	m_currentBackbufferIndex = m_swapchain->GetCurrentBackBufferIndex();
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

void Renderer::FlushGPU()
{
	HRESULT hr = m_directCmdQueue->Signal(m_fence, m_fenceValues[m_currentBackbufferIndex % m_numFramesInFlight]);
	assert(SUCCEEDED(hr));
	hr = m_fence->SetEventOnCompletion(m_fenceValues[m_currentBackbufferIndex % m_numFramesInFlight], m_eventHandle);
	assert(SUCCEEDED(hr));
	WaitForSingleObject(m_eventHandle, INFINITE);
	m_fenceValues[m_currentBackbufferIndex % m_numFramesInFlight]++;
}

void Renderer::FrameFence()
{
	UINT64 fenceValueThisFrame = m_fenceValues[m_currentBackbufferIndex % m_numFramesInFlight];
	m_currentBackbufferIndex = m_swapchain->GetCurrentBackBufferIndex();
	//this function should be called after present so m_currentBackbufferIndex now holds the next frames backbufferIndex
	UINT64 fenceIndexNextFrame = m_currentBackbufferIndex % m_numFramesInFlight;
	HRESULT hr = m_directCmdQueue->Signal(m_fence, fenceValueThisFrame);
	assert(SUCCEEDED(hr));

	if (m_fence->GetCompletedValue() < m_fenceValues[fenceIndexNextFrame])
	{
		hr = m_fence->SetEventOnCompletion(m_fenceValues[fenceIndexNextFrame], m_eventHandle);
		assert(SUCCEEDED(hr));
		WaitForSingleObject(m_eventHandle, INFINITE);
	}
	m_fenceValues[fenceIndexNextFrame] = fenceValueThisFrame + 1;
}


