#pragma once
#include "RenderPass.h"
class PostProcessingPass : public RenderPass
{
public:
	PostProcessingPass(ID3D12Device* device, int framesInFlight);
	~PostProcessingPass();

	void RunRenderPass(ID3D12GraphicsCommandList* cmdList, FrameResource& frameResource, int frameIndex) override;
	void RecreateOnResolutionChange(ID3D12Device* device, int framesInFlight, UINT width, UINT height) override;
	std::string Name() const override;
private:
	ID3D12Device* m_device = nullptr;
	ID3D12RootSignature* m_rootSignature = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;

};

