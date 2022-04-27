#include "pch.h"
#include "DescriptorVector.h"

DescriptorVector::DescriptorVector(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags, UINT maxSize)
{
	capacity = maxSize;
	heapDescriptorDesc.Flags = flags;
	heapDescriptorDesc.NodeMask = 0;
	heapDescriptorDesc.Type = type;
	heapDescriptorDesc.NumDescriptors = capacity;
}

DescriptorVector::~DescriptorVector()
{
	if (heapDescriptor) heapDescriptor->Release();
}

UINT DescriptorVector::CreateConstantBuffer(ID3D12Device* device, D3D12_CONSTANT_BUFFER_VIEW_DESC* buffer)
{
	assert(size < capacity);
	if (heapDescriptor == nullptr)
	{
		Init(device);
	}
	D3D12_CPU_DESCRIPTOR_HANDLE heapHandle = heapDescriptor->GetCPUDescriptorHandleForHeapStart();
	heapHandle.ptr += size * incrementSize;
	device->CreateConstantBufferView(buffer, heapHandle);

	return size++;
}

UINT DescriptorVector::CreateShaderResource(ID3D12Device* device, ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* viewDesc)
{
	assert(size < capacity);
	if (heapDescriptor == nullptr)
	{
		Init(device);
	}
	D3D12_CPU_DESCRIPTOR_HANDLE heapHandle = heapDescriptor->GetCPUDescriptorHandleForHeapStart();
	heapHandle.ptr += size * incrementSize;
	device->CreateShaderResourceView(resource, viewDesc, heapHandle);


	return size++;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorVector::operator[](UINT index) const
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = heapDescriptor->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += index * incrementSize;
	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorVector::GetGPUHandle(UINT index) const
{
	assert(heapDescriptorDesc.Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE && index < size);
	D3D12_GPU_DESCRIPTOR_HANDLE handle = heapDescriptor->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += index * incrementSize;
	return handle;
}

void DescriptorVector::Clear()
{
	size = 0;
}

void DescriptorVector::AddFromOther(const DescriptorVector& other, UINT srcIndex, UINT count, ID3D12Device* device)
{
	assert(heapDescriptorDesc.Type == other.heapDescriptorDesc.Type && srcIndex + count <= other.size);
	assert(other.Get());
	if (this->Get() == nullptr)
		this->Init(device);
	device->CopyDescriptorsSimple(count, this->operator[](size), other[srcIndex], heapDescriptorDesc.Type);
	size += count;
}

UINT DescriptorVector::Push(D3D12_CPU_DESCRIPTOR_HANDLE src, ID3D12Device* device)
{
	assert(this->Get() && this->size < capacity);
	device->CopyDescriptorsSimple(1, operator[](size), src, heapDescriptorDesc.Type);
	return size++;
}

ID3D12DescriptorHeap* DescriptorVector::Get() const
{
	return heapDescriptor;
}

void DescriptorVector::Init(ID3D12Device* device)
{
	if (heapDescriptor == nullptr)
	{
		HRESULT hr = device->CreateDescriptorHeap(&heapDescriptorDesc, __uuidof(ID3D12DescriptorHeap), (void**)&heapDescriptor);
		assert(SUCCEEDED(hr));

		incrementSize = device->GetDescriptorHandleIncrementSize(heapDescriptorDesc.Type);
	}
}

void DescriptorVector::Resize(UINT newsize)
{
	assert(newsize <= capacity); //allocate new heap if needed later on, for now capacity is fixed
	size = newsize;
}

UINT DescriptorVector::Size() const
{
	return size;
}
