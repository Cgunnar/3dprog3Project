#pragma once
#include "RenderPass.h"
class IndirectRenderPass : public RenderPass
{
public:
	IndirectRenderPass(ID3D12Device* device, int framesInFlight);
	~IndirectRenderPass();
	RenderPassRequirements GetRequirements() override;
	void RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex) override;
	void RecreateOnResolutionChange(ID3D12Device* device, int framesInFlight, UINT width, UINT height) override;
	std::string Name() const override;

private:
	int m_framesInFlight;
	ID3D12Device* m_device = nullptr;

	ID3D12RootSignature* m_rootSignature = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;

	ID3D12RootSignature* m_rootSignatureCompute = nullptr;
	ID3D12PipelineState* m_pipelineStateCompute = nullptr;

	void SetUpRenderPipeline(ID3DBlob* vs, ID3DBlob* ps);
	void SetUpComputePipeline(ID3DBlob* cs);
};

