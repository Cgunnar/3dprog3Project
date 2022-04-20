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
}

Renderer::~Renderer()
{
	m_swapchain->Release();
	m_fence->Release();
	m_directCmdList->Release();
	m_directCmdAllocator->Release();
	m_directCmdQueue->Release();
	m_device->Release();
	m_adapter->Release();
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