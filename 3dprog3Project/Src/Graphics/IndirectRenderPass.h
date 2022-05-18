#pragma once
#include "RenderPass.h"
class IndirectRenderPass : public RenderPass
{
public:
	IndirectRenderPass(ID3D12Device* device, int framesInFlight, DXGI_FORMAT renderTargetFormat);
	~IndirectRenderPass();
	RenderPassRequirements GetRequirements() override;
	void RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex) override;
	void RecreateOnResolutionChange(ID3D12Device* device, int framesInFlight, UINT width, UINT height) override;
	std::string Name() const override;

private:
	constexpr static size_t meshIndexRP = 0;
	constexpr static size_t vbBindlessRP = 1;
	constexpr static size_t ibBindlessRP = 2;

	DXGI_FORMAT m_rtFormat;
	int m_framesInFlight;
	ID3D12Device* m_device = nullptr;

	ID3D12RootSignature* m_rootSignature = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;

	ID3D12RootSignature* m_rootSignatureCompute = nullptr;
	ID3D12PipelineState* m_pipelineStateCompute = nullptr;

	void SetUpRenderPipeline(ID3DBlob* vs, ID3DBlob* ps);
	void SetUpComputePipeline(ID3DBlob* cs);
};

