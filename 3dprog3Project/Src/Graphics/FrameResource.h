#pragma once

class Renderer;
class FrameResource
{
	friend Renderer;
public:
	FrameResource(ID3D12Device* device, UINT width, UINT height);
	~FrameResource();
	FrameResource(const FrameResource& other) = delete;
	FrameResource& operator=(const FrameResource& other) = delete;
	FrameResource(FrameResource&& other) noexcept;
	FrameResource& operator=(FrameResource&& other) noexcept;

	std::pair<UINT, UINT> GetResolution() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetDsvCpuHandle() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetRtvCpuHandle() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetRtSrvCpuHandle() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferCpuHandle() const;
	ID3D12Resource* renderTarget = nullptr;
	ID3D12Resource* depthBuffer = nullptr;
private:
	ID3D12DescriptorHeap* m_dsvDescHeap = nullptr;
	ID3D12DescriptorHeap* m_rtvDescHeap = nullptr;
	ID3D12DescriptorHeap* m_rtAsSrvDescHeap = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_backBufferCpuDescHandle{};
	UINT m_rtvHeapDescIncremenSize = 0;
	UINT m_width;
	UINT m_height;
};

