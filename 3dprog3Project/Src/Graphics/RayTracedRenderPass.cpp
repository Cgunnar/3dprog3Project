#include "pch.h"
#include "RayTracedRenderPass.h"

RayTracedRenderPass::RayTracedRenderPass(ID3D12Device* device, int framesInFlight, DXGI_FORMAT renderTargetFormat)
	: MainRenderPass(device, framesInFlight, renderTargetFormat, 1)
{

}

RayTracedRenderPass::~RayTracedRenderPass()
{

}

RenderPassRequirements RayTracedRenderPass::GetRequirements()
{
	return MainRenderPass::GetRequirements();
}

void RayTracedRenderPass::RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex)
{
}

void RayTracedRenderPass::RecreateOnResolutionChange(ID3D12Device* device, int framesInFlight, UINT width, UINT height)
{
	return;
}

std::string RayTracedRenderPass::Name() const
{
	return "RayTracedRenderPass";
}
