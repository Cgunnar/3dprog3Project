#pragma once
#include "RenderPass.h"
#include "FrameResource.h"
#include "RenderingTypes.h"

class Window;
class Renderer
{
	friend Window;
public:
	Renderer(HWND windowHandle, RenderingSettings settings);
	~Renderer();
	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	UINT64 Render();
	ID3D12Device* GetDevice();
	void OnResize(UINT width, UINT height, bool forceResolution);
	bool SetFullscreen(bool fullscreen, UINT exitFullscreenWidth = 0, UINT exitFullscreenHeight = 0);
	DXGI_MODE_DESC GetBestDisplayMode();
	int GetNumberOfFramesInFlight() const;
	void DisplayChanged();
	RenderingSettings GetRenderingSettings() const;
	bool vsyncEnabled = false;
	void FlushGPU();
	std::pair<UINT, UINT> GetDisplayResolution() const;
private:
	UINT m_width;
	UINT m_height;
	bool m_fullscreen = false;
	BOOL m_hasVariableRefreshRate = false;
	DXGI_OUTPUT_DESC1 m_outputDesc;
	RenderingSettings m_renderingSettings;

	std::vector<std::unique_ptr<RenderPass>> m_renderPasses;

	HWND m_hWnd;
	ID3D12Device* m_device = nullptr;
	IDXGISwapChain3* m_swapchain = nullptr;
	IDXGIAdapter4* m_adapter = nullptr;
	ID3D12CommandQueue* m_directCmdQueue = nullptr;
	std::vector<ID3D12CommandAllocator*> m_directCmdAllocator;
	ID3D12GraphicsCommandList* m_directCmdList = nullptr;

	ID3D12Fence* m_fence = nullptr;
	std::vector<UINT64> m_fenceValues;
	HANDLE m_eventHandle;
	int m_numFramesInFlight;
	int m_numBackBuffers;

	UINT64 m_vramInBytes = 0;

	size_t m_currentBackbufferIndex = 0;
	std::vector<ID3D12Resource*> m_backbuffers;
	ID3D12DescriptorHeap* m_rtvDescHeap = nullptr;
	UINT m_rtvDescSize = 0;

	std::vector<FrameResource> m_frameResources;

	ID3D12DescriptorHeap* m_imguiDescHeap = nullptr;

	void BeginFrame();
	void EndFrame();
	void CreateDeviceAndDirectCmd(IDXGIFactory6* factory);
	void CreateSwapChain(IDXGIFactory5* factory, HWND windowHandle);
	void CreateRTV();

	
	void FrameFence();
	MemoryInfo GetVramInfo(IDXGIAdapter4* adapter);
	std::vector<DXGI_MODE_DESC> CheckMonitorRes();
	DXGI_OUTPUT_DESC1 GetOutputCapabilities() const;
};

