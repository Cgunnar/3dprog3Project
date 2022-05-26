#pragma once
template<typename T> requires std::is_trivially_copyable_v<T>
class StructuredBuffer
{
public:
	StructuredBuffer(ID3D12Device* device, UINT maxNumElements, bool createView, bool cpuWrite)
		: m_device(device), m_cpuWrite(cpuWrite), m_maxNumElements(maxNumElements)
	{
		D3D12_RESOURCE_DESC desc;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.MipLevels = 1;
		desc.DepthOrArraySize = 1;
		desc.Height = 1;
		desc.Width = sizeof(T) * maxNumElements;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		if (cpuWrite)
		{
			D3D12_HEAP_PROPERTIES heapProps{};
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
			HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
				&desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&m_buffer));
			assert(SUCCEEDED(hr));
		}
		else
		{
			D3D12_HEAP_PROPERTIES heapProps{};
			heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
			HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
				&desc, D3D12_RESOURCE_STATE_COMMON, nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&m_buffer));
			assert(SUCCEEDED(hr));
		}


		if (createView)
		{
			D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
			descHeapDesc.NodeMask = 0;
			descHeapDesc.NumDescriptors = 1;
			descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

			HRESULT hr = device->CreateDescriptorHeap(&descHeapDesc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void**>(&m_descriptorHeap));
			assert(SUCCEEDED(hr));

			
		}
	}

	StructuredBuffer(const StructuredBuffer& other) = delete;
	StructuredBuffer& operator=(const StructuredBuffer& other) = delete;

	~StructuredBuffer()
	{
		m_buffer->Release();
		if (m_descriptorHeap) m_descriptorHeap->Release();
	}

	void Update(const T* data, UINT count)
	{
		//assert(count <= m_maxNumElements && count > 0);
		if (m_cpuWrite)
		{
			if (m_descriptorHeap)
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc;
				viewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				viewDesc.Format = DXGI_FORMAT_UNKNOWN;
				viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				viewDesc.Buffer.FirstElement = 0;
				viewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
				viewDesc.Buffer.NumElements = count;
				viewDesc.Buffer.StructureByteStride = sizeof(T);

				D3D12_CPU_DESCRIPTOR_HANDLE heapHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
				m_device->CreateShaderResourceView(m_buffer, &viewDesc, heapHandle);
			}


			void* mappedMemory = nullptr;
			D3D12_RANGE emptyRange = { 0, 0 };
			HRESULT hr = m_buffer->Map(0, &emptyRange, &mappedMemory);
			assert(SUCCEEDED(hr));
			std::memcpy(mappedMemory, data, sizeof(T) * count);
			m_buffer->Unmap(0, nullptr);
		}
		else
		{
			assert(false);
		}
	}

	ID3D12Resource* Get() const
	{
		return m_buffer;
	}

	D3D12_GPU_VIRTUAL_ADDRESS GpuAddress() const
	{
		return m_buffer->GetGPUVirtualAddress();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle() const
	{
		if (m_descriptorHeap)
			return m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		else
		{
			assert(false);
			return D3D12_CPU_DESCRIPTOR_HANDLE{};
		}
	}

private:
	UINT m_maxNumElements;
	ID3D12Device* m_device = nullptr;
	ID3D12Resource* m_buffer = nullptr;
	ID3D12DescriptorHeap* m_descriptorHeap = nullptr;
	bool m_cpuWrite;
};