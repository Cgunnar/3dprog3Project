#pragma once
#include "RenderPass.h"
#include "ConstantBufferManager.h"
class ZPreRenderPass : public RenderPass
{
public:
	ZPreRenderPass(RenderingSettings settings, ID3D12Device* device, DXGI_FORMAT renderTargetFormat);
	~ZPreRenderPass();
	RenderPassRequirements GetRequirements() override;
	void Start(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) override;
	void SubmitObjectsToRender(const std::vector<RenderUnit>& renderUnits) override;
	void RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex) override;
	bool OnRenderingSettingsChange(RenderingSettings settings, ID3D12Device* device) override;
	std::string Name() const override;
private:
	ID3D12Device* m_device = nullptr;
	ID3D12RootSignature* m_rootSignature = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;
	DXGI_FORMAT m_rtFormat;
	std::vector<RenderUnit> m_renderUnits;
	std::vector<ConstantBufferManager*> m_constantBuffers;
};