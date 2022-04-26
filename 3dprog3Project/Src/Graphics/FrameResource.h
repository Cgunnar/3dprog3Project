#pragma once
class FrameResource
{
public:
	FrameResource(ID3D12Device* device, UINT width, UINT height);
	~FrameResource();
	FrameResource(const FrameResource& other) = delete;
	FrameResource& operator=(const FrameResource& other) = delete;
	FrameResource(FrameResource&& other) noexcept;
	FrameResource& operator=(FrameResource&& other) noexcept;

	std::pair<UINT, UINT> GetResolution() const;
	
	ID3D12Resource* renderTarget = nullptr;
	ID3D12Resource* depthBuffer = nullptr;
private:
	ID3D12DescriptorHeap* m_dsvDescHeap = nullptr;
	ID3D12DescriptorHeap* m_rtvDescHeap = nullptr;
	UINT m_rtvHeapDescIncremenSize = 0;
	UINT m_width;
	UINT m_height;
};

