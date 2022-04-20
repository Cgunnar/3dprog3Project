#pragma once



class Renderer
{
public:
	Renderer(HWND windowHandle);
	~Renderer();
	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	UINT64 Render();

private:
	ID3D12Device* m_device = nullptr;
	IDXGISwapChain3* m_swapchain = nullptr;
	IDXGIAdapter4* m_adapter = nullptr;
	ID3D12CommandQueue* m_directCmdQueue = nullptr;
	ID3D12CommandAllocator* m_directCmdAllocator = nullptr;
	ID3D12GraphicsCommandList* m_directCmdList = nullptr;

	ID3D12Fence* m_fence = nullptr;
	UINT64 m_fenceValue = 0;
	UINT64 m_vramInBytes = 0;

	int m_numFramesInFlight = 1;

	size_t m_currentBackbufferIndex = 0;
	std::array<ID3D12Resource*, 2> m_backbuffers;
	ID3D12DescriptorHeap* m_rtvDescHeap = nullptr;
	UINT m_rtvDescSize = 0;

	ID3D12Resource* m_depthBufferResource = nullptr;
	ID3D12DescriptorHeap* m_dsvDescHeap = nullptr;
	ID3D12DescriptorHeap* m_imguiDescHeap = nullptr;


	void BeginFrame();
	void EndFrame();
	void CreateDeviceAndDirectCmd(IDXGIFactory6* factory);
	void CreateSwapChain(IDXGIFactory2* factory, HWND windowHandle);
	void FrameFence();
	MemoryInfo GetVramInfo(IDXGIAdapter4* adapter);
};

