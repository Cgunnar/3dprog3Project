#pragma once
#include "FrameResource.h"
#include "DescriptorPool.h"

struct RenderPassRequirements
{
	UINT cmdListCount;
	UINT numDescriptorHandles;
	UINT descriptorHandleSize;
};

class RenderPass
{
public:
	RenderPass() = default;
	virtual ~RenderPass() = default;
	virtual RenderPassRequirements GetRequirements() = 0;
	virtual void Start(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) = 0;
	virtual void RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex) = 0;
	virtual std::string Name() const = 0;
	virtual void RecreateOnResolutionChange(ID3D12Device* device, int framesInFlight, UINT width, UINT height) = 0;
};

ID3DBlob* LoadCSO(const std::string& filepath);

