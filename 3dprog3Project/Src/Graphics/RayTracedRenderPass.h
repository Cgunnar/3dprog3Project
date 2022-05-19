#pragma once
#include "MainRenderPass.h"
class RayTracedRenderPass : public MainRenderPass
{
public:
	RayTracedRenderPass(ID3D12Device* device, int framesInFlight, DXGI_FORMAT renderTargetFormat);
	virtual ~RayTracedRenderPass();
	virtual RenderPassRequirements GetRequirements() override;
	virtual void RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex) override;
	virtual void RecreateOnResolutionChange(ID3D12Device* device, int framesInFlight, UINT width, UINT height) override;
	virtual std::string Name() const override;
};

