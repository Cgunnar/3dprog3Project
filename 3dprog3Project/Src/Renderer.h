#pragma once



class Renderer
{
public:
	Renderer(HWND windowHandle);
	~Renderer();
	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

private:

	ID3D12Device* m_device = nullptr;
	IDXGISwapChain3* m_swapchain = nullptr;
	IDXGIAdapter4* m_adapter = nullptr;
	ID3D12CommandQueue* m_directCmdQueue = nullptr;
	ID3D12CommandAllocator* m_directCmdAllocator = nullptr;
	ID3D12GraphicsCommandList* m_directCmdList = nullptr;

	ID3D12Fence* m_fence = nullptr;
	UINT64 m_fenceValue = 0u;
	UINT64 m_vramInBytes = 0;

	size_t m_currentBackbufferIndex = 0;
	std::array<ID3D12Resource*, 2> m_backbuffers;

	void CreateDeviceAndDirectCmd(IDXGIFactory6* factory);
	void CreateSwapChain(IDXGIFactory2* factory, HWND windowHandle);
	MemoryInfo GetVramInfo(IDXGIAdapter4* adapter);
};

