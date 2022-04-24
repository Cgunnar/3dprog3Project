#pragma once

#include "DescriptorVector.h"
class ConstantBufferManager
{
public:
	ConstantBufferManager(ID3D12Device* device, UINT numConstantBuffers);
	~ConstantBufferManager();

private:
	ID3D12Resource* m_constantBufferPool = nullptr;
	std::byte* m_beginPtr = nullptr;
	std::byte* m_currentPtr = nullptr;
	std::byte* m_endPtr = nullptr;
	UINT m_maxNumConstantBuffers = 0;

	DescriptorVector* m_heapDescriptors = nullptr;
};

