#pragma once

class DescriptorVector
{
public:
	DescriptorVector(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT maxSize = 2048 * 4);
	~DescriptorVector();
	DescriptorVector(const DescriptorVector& other) = delete;
	DescriptorVector& operator=(const DescriptorVector& other) = delete;

	UINT CreateConstantBuffer(ID3D12Device* device, D3D12_CONSTANT_BUFFER_VIEW_DESC* buffer);
	UINT CreateShaderResource(ID3D12Device* device, ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* viewDesc = nullptr);

	D3D12_CPU_DESCRIPTOR_HANDLE operator[](UINT index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(UINT index) const;
	void Clear();
	void AddFromOther(const DescriptorVector& other, UINT srcIndex, UINT count, ID3D12Device* device);
	UINT Push(D3D12_CPU_DESCRIPTOR_HANDLE src, ID3D12Device* device);
	ID3D12DescriptorHeap* Get() const;
	void Init(ID3D12Device* device);
	void Resize(UINT newsize);
	UINT Size() const;

private:
	UINT size = 0;
	UINT capacity = 0;
	UINT incrementSize = 0;
	ID3D12DescriptorHeap* heapDescriptor = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC heapDescriptorDesc;
};

