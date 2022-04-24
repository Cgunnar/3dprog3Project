#pragma once
#include "RenderPass.h"
#include "DescriptorVector.h"
#include "ConstantBufferManager.h"
class TestRenderPass : public RenderPass
{
public:
	TestRenderPass(ID3D12Device* device, int framesInFlight);
	~TestRenderPass();

	void RunRenderPass(ID3D12GraphicsCommandList* cmdList, int frameIndex) override;
private:
	ID3D12Device* m_device = nullptr;
	ID3D12RootSignature* m_rootSignature = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;
	DescriptorVector m_heapDescriptor = DescriptorVector(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	std::vector<ConstantBufferManager*> m_constantBuffers;
};

