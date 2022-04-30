#include "pch.h"
#include "Renderer.h"
#include "TestRenderPass.h"
#include "PostProcessingPass.h"
#include "pix3.h"
#include <imgui_impl_dx12.h>

#pragma comment( lib, "d3d12.lib")
#pragma comment( lib, "d3dcompiler.lib")
#pragma comment( lib, "dxgi.lib")

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

Renderer::Renderer(HWND windowHandle, RenderingSettings settings) : m_hWnd(windowHandle), m_renderingSettings(settings)
{
	m_numBackBuffers = settings.numberOfBackbuffers;
	m_numFramesInFlight = settings.numberOfFramesInFlight;
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


	int maxCountOfListPerFram = 12;
	m_3dRenderPassesCmdLists.resize(maxCountOfListPerFram);
	m_3dRenderPassesCmdAllocators.resize(maxCountOfListPerFram);
	for (int i = 0; i < maxCountOfListPerFram; i++)
	{
		auto& cmdAllocators = m_3dRenderPassesCmdAllocators[i];
		cmdAllocators.resize(m_numFramesInFlight);
		for (int j = 0; j < cmdAllocators.size(); j++)
		{
			hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), reinterpret_cast<void**>(&cmdAllocators[j]));
			assert(SUCCEEDED(hr));
			std::wstring name = std::wstring(L"recorderCmdAllocator number: ") + std::to_wstring(i) + L"fameID: " + std::to_wstring(j);
			hr = cmdAllocators[j]->SetName(name.c_str());
			assert(SUCCEEDED(hr));
		}

		auto& cmdList = m_3dRenderPassesCmdLists[i];
		hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocators.front(), nullptr, __uuidof(ID3D12GraphicsCommandList), reinterpret_cast<void**>(&cmdList));
		assert(SUCCEEDED(hr));
		hr = cmdList->Close();
		assert(SUCCEEDED(hr));

		std::wstring name = std::wstring(L"recorderCmdList number: ") + std::to_wstring(i);
		hr = cmdList->SetName(name.c_str());
		assert(SUCCEEDED(hr));
	}
	
	factory6->Release();

	m_desriptorPool = std::make_unique<DescriptorPool>(m_device, m_numFramesInFlight,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	for (int i = 0; i < m_numFramesInFlight; i++)
	{
		m_frameResources.emplace_back(m_device, settings.renderWidth, settings.renderHeight);
	}

	CheckMonitorRes();

	m_fenceValues.resize(m_numFramesInFlight, 0);
	hr = m_device->CreateFence(m_fenceValues[m_currentBackbufferIndex % m_numFramesInFlight], D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), reinterpret_cast<void**>(&m_fence));
	assert(SUCCEEDED(hr));
	m_fenceValues[m_currentBackbufferIndex % m_numFramesInFlight]++;

	m_eventHandle = CreateEventEx(nullptr, 0, 0, EVENT_ALL_ACCESS);
	assert(m_eventHandle);


	D3D12_DESCRIPTOR_HEAP_DESC rtvDescHDesc;
	rtvDescHDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescHDesc.NumDescriptors = static_cast<UINT>(m_backbuffers.size());
	rtvDescHDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvDescHDesc.NodeMask = 0;

	m_rtvDescSize = m_device->GetDescriptorHandleIncrementSize(rtvDescHDesc.Type);
	hr = m_device->CreateDescriptorHeap(&rtvDescHDesc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(&m_rtvDescHeap));
	assert(SUCCEEDED(hr));

	CreateRTV();

	m_renderPasses.emplace_back(std::make_unique<TestRenderPass>(m_device, m_numFramesInFlight));
	m_renderPasses.emplace_back(std::make_unique<PostProcessingPass>(m_device, m_numFramesInFlight));

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

	m_rtvDescHeap->Release();
	for (auto& bb : m_backbuffers)
		bb->Release();
	m_swapchain->Release();
	m_fence->Release();

	for (auto& cmdList : m_3dRenderPassesCmdLists)
		cmdList->Release();
	for (auto& cmdAllocators : m_3dRenderPassesCmdAllocators)
	{
		for (auto& allocatorPerFrame : cmdAllocators)
			allocatorPerFrame->Release();
	}

	m_directCmdListStart->Release();
	for (auto& dca : m_directCmdAllocatorStart)
		dca->Release();
	m_directCmdListEnd->Release();
	for (auto& dca : m_directCmdAllocatorEnd)
		dca->Release();
	m_directCmdQueue->Release();
	m_device->Release();
	m_adapter->Release();

	CloseHandle(m_eventHandle);
}

ID3D12Device* Renderer::GetDevice()
{
	return m_device;
}

void Renderer::BeginFrame()
{
	size_t frameIndex = m_currentBackbufferIndex % m_numFramesInFlight;
	HRESULT hr = m_directCmdAllocatorStart[frameIndex]->Reset();
	assert(SUCCEEDED(hr));
	hr = m_directCmdListStart->Reset(m_directCmdAllocatorStart[frameIndex], nullptr);
	assert(SUCCEEDED(hr));

	hr = m_directCmdAllocatorEnd[frameIndex]->Reset();
	assert(SUCCEEDED(hr));
	hr = m_directCmdListEnd->Reset(m_directCmdAllocatorEnd[frameIndex], nullptr);
	assert(SUCCEEDED(hr));


	
	D3D12_CPU_DESCRIPTOR_HANDLE backBufferHandle = m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
	backBufferHandle.ptr += m_rtvDescSize * m_currentBackbufferIndex;

	D3D12_RESOURCE_BARRIER backbufferTransitionBarrier;
	backbufferTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	backbufferTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	backbufferTransitionBarrier.Transition.pResource = m_backbuffers[m_currentBackbufferIndex];
	backbufferTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	backbufferTransitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	backbufferTransitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	m_directCmdListStart->ResourceBarrier(1, &backbufferTransitionBarrier);

	m_frameResources[frameIndex].m_backBufferCpuDescHandle = backBufferHandle;

	auto descripterHeap = m_desriptorPool->Get();
	m_directCmdListStart->SetDescriptorHeaps(1, &descripterHeap);
	m_directCmdListEnd->SetDescriptorHeaps(1, &descripterHeap);
	m_desriptorPool->SetNextFrame();
}

size_t Renderer::Render()
{
	int frameIndex = m_currentBackbufferIndex % m_numFramesInFlight;
	m_activeRenderPassCmdListsCount = 0;
	BeginFrame();
	
	for (auto& renderPass : m_renderPasses)
	{
		auto req = renderPass->GetRequirements();
		std::vector<DescriptorHandle> handles;
		for (int i = 0; i < req.numDescriptorHandles; i++)
		{
			handles.push_back(m_desriptorPool->DynamicAllocate(req.descriptorHandleSize));
		}
		assert(m_activeRenderPassCmdListsCount + req.cmdListCount < m_3dRenderPassesCmdLists.size());
		std::vector<ID3D12GraphicsCommandList*> cmdLists(
			m_3dRenderPassesCmdLists.begin() + m_activeRenderPassCmdListsCount,
			m_3dRenderPassesCmdLists.begin() + m_activeRenderPassCmdListsCount + req.cmdListCount);

		for (int i = 0; i < req.cmdListCount; i++)
		{
			auto& cmdAlloc = m_3dRenderPassesCmdAllocators[m_activeRenderPassCmdListsCount + i][frameIndex];
			HRESULT hr = cmdAlloc->Reset();
			assert(SUCCEEDED(hr));
			hr = cmdLists[i]->Reset(cmdAlloc, nullptr);
			assert(SUCCEEDED(hr));

			auto descripterHeap = m_desriptorPool->Get();
			cmdLists[i]->SetDescriptorHeaps(1, &descripterHeap);
		}
		m_activeRenderPassCmdListsCount += req.cmdListCount;

		//PIXBeginEvent(m_directCmdList, 200, renderPass->Name().c_str());
		renderPass->RunRenderPass(cmdLists, handles, m_frameResources[frameIndex], frameIndex);
		//PIXEndEvent(m_directCmdList);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_frameResources[frameIndex].GetBackBufferCpuHandle();
	m_directCmdListEnd->OMSetRenderTargets(1, &rtvHandle, true, nullptr);
	m_directCmdListEnd->SetDescriptorHeaps(1, &m_imguiDescHeap);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_directCmdListEnd);
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

	m_directCmdListEnd->ResourceBarrier(1, &backbufferTransitionBarrier);

	HRESULT hr = m_directCmdListStart->Close();
	assert(SUCCEEDED(hr));
	hr = m_directCmdListEnd->Close();
	assert(SUCCEEDED(hr));
	std::vector<ID3D12CommandList*> tmpCmdList;
	tmpCmdList.push_back(m_directCmdListStart);
	for (int i = 0; i < m_activeRenderPassCmdListsCount; i++)
	{
		hr = m_3dRenderPassesCmdLists[i]->Close();
		assert(SUCCEEDED(hr));
		tmpCmdList.push_back(m_3dRenderPassesCmdLists[i]);
	}
	tmpCmdList.push_back(m_directCmdListEnd);

	m_directCmdQueue->ExecuteCommandLists(tmpCmdList.size(), tmpCmdList.data());
	//for (auto& cmdList : tmpCmdList)
	//{
	//	//Sleep(1);
	//	m_directCmdQueue->ExecuteCommandLists(1, &cmdList);
	//}


	// this comment is from microsofts smmple repo
	// When using sync interval 0, it is recommended to always pass the tearing
	// flag when it is supported, even when presenting in windowed mode.
	// However, this flag cannot be used if the app is in fullscreen mode as a
	// result of calling SetFullscreenState.
	
	
	if(m_renderingSettings.vsync)
	{
		hr = m_swapchain->Present(1, 0);
		assert(SUCCEEDED(hr));
	}
	else
	{
		hr = m_swapchain->Present(0, !m_fullscreen && m_hasVariableRefreshRate ? DXGI_PRESENT_ALLOW_TEARING : 0); // this is for variable rate displayes
		assert(SUCCEEDED(hr));
	}
	

	FrameFence();
}

void Renderer::OnResize(UINT width, UINT height, bool forceResolution)
{
	//if (width == m_width && height == m_height) return;

	FlushGPU();

	for (auto& fv : m_fenceValues)
		fv = m_fenceValues[m_currentBackbufferIndex % m_numFramesInFlight];

	for (auto& bb : m_backbuffers)
		bb->Release();

	DXGI_SWAP_CHAIN_DESC1 swapDesc;
	HRESULT hr = m_swapchain->GetDesc1(&swapDesc);
	assert(SUCCEEDED(hr));

	if (forceResolution)
	{
		hr = m_swapchain->ResizeBuffers(0, width, height, swapDesc.Format, swapDesc.Flags);
		assert(SUCCEEDED(hr));
	}
	else
	{
		hr = m_swapchain->ResizeBuffers(0, 0, 0, swapDesc.Format, swapDesc.Flags);
		assert(SUCCEEDED(hr));
	}

	m_currentBackbufferIndex = m_swapchain->GetCurrentBackBufferIndex();
	hr = m_swapchain->GetDesc1(&swapDesc);
	assert(SUCCEEDED(hr));
	m_width = swapDesc.Width;
	m_height = swapDesc.Height;

	utl::PrintDebug("Swapchain updated to width: " + std::to_string(m_width) + ", height: " + std::to_string(m_height));

	CreateRTV();
}

bool Renderer::SetFullscreen(bool fullscreen, UINT exitFullscreenWidth, UINT exitFullscreenHeight)
{
	BOOL currentState;
	HRESULT hr = m_swapchain->GetFullscreenState(&currentState, nullptr);
	if (currentState == fullscreen)
	{
		utl::PrintDebug("Renderer::SetFullscreen state mismatch");
		m_fullscreen = currentState;
		return m_fullscreen;
	}

	if (fullscreen)
	{
		DXGI_MODE_DESC modeDesc = CheckMonitorRes().front();
		HRESULT hr = m_swapchain->ResizeTarget(&modeDesc);
		assert(SUCCEEDED(hr));
		hr = m_swapchain->SetFullscreenState(TRUE, nullptr);
		assert(SUCCEEDED(hr));
		m_fullscreen = true;

		hr = m_swapchain->ResizeTarget(&modeDesc);
		assert(SUCCEEDED(hr));
		OnResize(modeDesc.Width, modeDesc.Height, true);
		return true;
	}
	else
	{
		DXGI_MODE_DESC modeDesc;
		modeDesc.Width = exitFullscreenWidth;
		modeDesc.Height = exitFullscreenHeight;

		if (modeDesc.Width == 0 || modeDesc.Height == 0)
		{
			DXGI_SWAP_CHAIN_DESC1 swapDesc;
			HRESULT hr = m_swapchain->GetDesc1(&swapDesc);
			assert(SUCCEEDED(hr));
			modeDesc.Width = swapDesc.Width;
			modeDesc.Height = swapDesc.Height;
		}
		hr = m_swapchain->SetFullscreenState(FALSE, nullptr);
		assert(SUCCEEDED(hr));
		
		auto displayRect = GetOutputCapabilities().DesktopCoordinates;
		MoveWindow(m_hWnd, displayRect.left, displayRect.top, modeDesc.Width, modeDesc.Height, false);
		ShowWindow(m_hWnd, SW_NORMAL);
		//MoveWindow looks a litle bit nicer
		/*HRESULT hr = m_swapchain->ResizeTarget(&modeDesc);
		assert(SUCCEEDED(hr));
		OnResize(modeDesc.Width, modeDesc.Height, false);*/
		m_fullscreen = false;
		return false;
	}
}

DXGI_MODE_DESC Renderer::GetBestDisplayMode()
{
	return CheckMonitorRes().front();
}

int Renderer::GetNumberOfFramesInFlight() const
{
	return m_numFramesInFlight;
}

void Renderer::DisplayChanged()
{
	IDXGIFactory6* factory;
	HRESULT hr = m_swapchain->GetParent(__uuidof(IDXGIFactory6), reinterpret_cast<void**>(&factory));
	assert(SUCCEEDED(hr));
	hr = factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &m_hasVariableRefreshRate, sizeof(BOOL));
	assert(SUCCEEDED(hr));
	factory->Release();
	utl::PrintDebug("Display has VariableRefreshRate: " + std::to_string(m_hasVariableRefreshRate));

	m_currentBackbufferIndex = m_swapchain->GetCurrentBackBufferIndex();
	m_outputDesc = GetOutputCapabilities();
	OnResize(0, 0, false);
	
}

RenderingSettings Renderer::GetRenderingSettings() const
{
	return m_renderingSettings;
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


	D3D12_FEATURE_DATA_D3D12_OPTIONS featureSupport{};

	hr = m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &featureSupport, sizeof(featureSupport));
	assert(SUCCEEDED(hr));
	switch (featureSupport.ResourceBindingTier)
	{
	case D3D12_RESOURCE_BINDING_TIER_1:
	{
		// Tier 1 is supported.
		utl::PrintDebug("Tier 1 is supported. Get new computer");
		break;
	}
	case D3D12_RESOURCE_BINDING_TIER_2:
	{
		// Tiers 1 and 2 are supported.
		utl::PrintDebug("Tiers 1 and 2 are supported. Get new computer");
		break;
	}
	case D3D12_RESOURCE_BINDING_TIER_3:
	{
		// Tiers 1, 2, and 3 are supported.
		utl::PrintDebug("Tiers 1, 2, and 3 are supported.");
		break;
	}
	}


	D3D12_COMMAND_QUEUE_DESC desc;
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = 0;
	desc.NodeMask = 0u;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	hr = m_device->CreateCommandQueue(&desc, __uuidof(ID3D12CommandQueue), reinterpret_cast<void**>(&m_directCmdQueue));
	assert(SUCCEEDED(hr));

	m_directCmdAllocatorStart.resize(m_numFramesInFlight, nullptr);
	for (int i = 0; i < m_directCmdAllocatorStart.size(); i++)
	{
		hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), reinterpret_cast<void**>(&m_directCmdAllocatorStart[i]));
		assert(SUCCEEDED(hr));
	}
	m_directCmdAllocatorEnd.resize(m_numFramesInFlight, nullptr);
	for (int i = 0; i < m_directCmdAllocatorEnd.size(); i++)
	{
		hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), reinterpret_cast<void**>(&m_directCmdAllocatorEnd[i]));
		assert(SUCCEEDED(hr));
	}

	hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_directCmdAllocatorStart.front(), nullptr, __uuidof(ID3D12GraphicsCommandList), reinterpret_cast<void**>(&m_directCmdListStart));
	assert(SUCCEEDED(hr));
	hr = m_directCmdListStart->Close();
	assert(SUCCEEDED(hr));

	hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_directCmdAllocatorEnd.front(), nullptr, __uuidof(ID3D12GraphicsCommandList), reinterpret_cast<void**>(&m_directCmdListEnd));
	assert(SUCCEEDED(hr));
	hr = m_directCmdListEnd->Close();
	assert(SUCCEEDED(hr));

	hr = m_directCmdQueue->SetName(L"mainCmdQueue");
	assert(SUCCEEDED(hr));
	for (int i = 0; i < m_directCmdAllocatorStart.size(); i++)
	{
		std::wstring name = std::wstring(L"mainCmdAllocatorStart: ") + std::to_wstring(i);
		hr = m_directCmdAllocatorStart[i]->SetName(name.c_str());
		assert(SUCCEEDED(hr));
	}
	for (int i = 0; i < m_directCmdAllocatorEnd.size(); i++)
	{
		std::wstring name = std::wstring(L"mainCmdAllocatorEnd: ") + std::to_wstring(i);
		hr = m_directCmdAllocatorEnd[i]->SetName(name.c_str());
		assert(SUCCEEDED(hr));
	}
	hr = m_directCmdListStart->SetName(L"mainCmdListStart");
	assert(SUCCEEDED(hr));
	hr = m_directCmdListEnd->SetName(L"mainCmdListEnd");
	assert(SUCCEEDED(hr));
}

void Renderer::CreateSwapChain(IDXGIFactory5* factory, HWND windowHandle)
{
	HRESULT hr = factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &m_hasVariableRefreshRate, sizeof(BOOL));
	utl::PrintDebug("Display has VariableRefreshRate: " + std::to_string(m_hasVariableRefreshRate) + "this always seems to be true even when it should not");
	m_backbuffers.resize(m_numBackBuffers);
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
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
	//it does not seem to be illigal to allow tearing on non VariableRefreshRate displayes
	//if (m_hasVariableRefreshRate) desc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	hr = factory->CreateSwapChainForHwnd(m_directCmdQueue, windowHandle, &desc, nullptr,
		nullptr, reinterpret_cast<IDXGISwapChain1**>(&m_swapchain));
	assert(SUCCEEDED(hr));

	m_swapchain->SetMaximumFrameLatency(m_numBackBuffers);

	hr = factory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER);
	assert(SUCCEEDED(hr));

	m_currentBackbufferIndex = m_swapchain->GetCurrentBackBufferIndex();

	hr = m_swapchain->GetDesc1(&desc);
	assert(SUCCEEDED(hr));
	m_width = desc.Width;
	m_height = desc.Height;

	m_outputDesc = GetOutputCapabilities();
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

std::pair<UINT, UINT> Renderer::GetDisplayResolution() const
{
	auto monitorRect = GetOutputCapabilities().DesktopCoordinates;
	return std::make_pair(abs(monitorRect.right - monitorRect.left), abs(monitorRect.top - monitorRect.bottom));
}

void Renderer::SetRenderResolution(UINT width, UINT height)
{
	FlushGPU();

	m_renderingSettings.renderWidth = width;
	m_renderingSettings.renderHeight = height;
	m_frameResources.clear();
	for (int i = 0; i < m_numFramesInFlight; i++)
	{
		m_frameResources.emplace_back(m_device, width, height);
	}

	for (auto& rp : m_renderPasses)
	{
		//rip object oriented programming
		//virual Destructors are fine but how do i create a new object of right type
		rp->RecreateOnResolutionChange(m_device, m_numFramesInFlight, width, height);
	}
}

void Renderer::SetVSync(bool value)
{
	m_renderingSettings.vsync = value;
}

void Renderer::CreateRTV()
{
	D3D12_CPU_DESCRIPTOR_HANDLE heapHandle;
	heapHandle = m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_backbuffers.size(); i++)
	{
		HRESULT hr = m_swapchain->GetBuffer(i, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&m_backbuffers[i]));
		assert(SUCCEEDED(hr));
		m_device->CreateRenderTargetView(m_backbuffers[i], nullptr, heapHandle);
		heapHandle.ptr += m_rtvDescSize;
	}
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

std::vector<DXGI_MODE_DESC> Renderer::CheckMonitorRes()
{
	IDXGIOutput* outPut;
	HRESULT hr = m_swapchain->GetContainingOutput(&outPut);
	assert(SUCCEEDED(hr));
	DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM;
	UINT numModes = 0;
	hr = outPut->GetDisplayModeList(format, 0, &numModes, 0);
	assert(SUCCEEDED(hr));
	std::vector<DXGI_MODE_DESC> modeVec;
	modeVec.resize(numModes);
	hr = outPut->GetDisplayModeList(format, 0, &numModes, modeVec.data());
	assert(SUCCEEDED(hr));

	std::sort(modeVec.begin(), modeVec.end(), [](auto a, auto b) {
		if (a.Width * a.Height > b.Width * b.Height) return true;
		if (a.Width * a.Height < b.Width * b.Height) return false;
		return (float)(a.RefreshRate.Numerator / (float)a.RefreshRate.Denominator) >
			(float)(b.RefreshRate.Numerator / (float)b.RefreshRate.Denominator);
		});

	

#ifdef _DEBUG
	UINT monitorWidth = modeVec.front().Width;
	UINT monitorHeight = modeVec.front().Height;
	float hz = (float)modeVec.front().RefreshRate.Numerator / (float)modeVec.front().RefreshRate.Denominator;
	DXGI_OUTPUT_DESC outDesc;
	outPut->GetDesc(&outDesc);
	std::wstring name = outDesc.DeviceName;
	std::wstring debugOut = name + L" Best Mode: resolution: " + std::to_wstring(monitorWidth) + L"x" + std::to_wstring(monitorHeight) +
		L" hz: " + std::to_wstring(hz) + L"\n";
	std::wcout << debugOut;
#endif // _DEBUG

	outPut->Release();
	assert(modeVec.front().ScanlineOrdering == DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE || modeVec.front().ScanlineOrdering == DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED);
	assert(modeVec.front().Scaling == DXGI_MODE_SCALING_STRETCHED || modeVec.front().Scaling == DXGI_MODE_SCALING_UNSPECIFIED);
	assert(modeVec.front().Format == DXGI_FORMAT_B8G8R8A8_UNORM);
	return modeVec;
}

DXGI_OUTPUT_DESC1 Renderer::GetOutputCapabilities() const
{
	IDXGIOutput* output;
	HRESULT hr = m_swapchain->GetContainingOutput(&output);
	assert(SUCCEEDED(hr));
	IDXGIOutput6* output6;
	hr = output->QueryInterface(__uuidof(IDXGIOutput6), reinterpret_cast<void**>(&output6));
	assert(SUCCEEDED(hr));

	DXGI_OUTPUT_DESC1 desc;
	hr = output6->GetDesc1(&desc);
	assert(SUCCEEDED(hr));
	output->Release();
	output6->Release();
	return desc;
}

