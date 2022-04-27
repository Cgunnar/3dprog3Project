#pragma once
#include "FrameResource.h"

class RenderPass
{
public:
	RenderPass() = default;
	virtual ~RenderPass() = default;
	virtual void RunRenderPass(ID3D12GraphicsCommandList* cmdList, FrameResource& frameResource, int frameIndex) = 0;
	virtual std::string Name() const = 0;
};

ID3DBlob* LoadCSO(const std::string& filepath);

