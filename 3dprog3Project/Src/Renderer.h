#pragma once
class Renderer
{
public:
	Renderer();
	~Renderer();
	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

private:

	ID3D12Device* device = nullptr;
	IDXGISwapChain3* swapchain = nullptr;
	IDXGIAdapter4* adapter = nullptr;
	ID3D12CommandQueue* commandQueue = nullptr;
	ID3D12CommandAllocator* commandAllocator = nullptr;
	ID3D12GraphicsCommandList* commandList = nullptr;

	ID3D12Fence* fence = nullptr;
	UINT64 fenceValue = 0u;
};

