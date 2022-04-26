#pragma once
#include "RenderPass.h"
class PostProcessingPass : public RenderPass
{
public:
	PostProcessingPass(ID3D12Device* device, int framesInFlight);
	~PostProcessingPass();

	void RunRenderPass(ID3D12GraphicsCommandList* cmdList, int frameIndex) override;
	std::string Name() const override;
private:
	ID3D12Device* m_device = nullptr;
	ID3D12RootSignature* m_rootSignature = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;
};

