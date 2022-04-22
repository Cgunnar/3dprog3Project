#pragma once
class RenderPass
{
public:
	RenderPass() = default;
	virtual ~RenderPass() = default;
	virtual void RunRenderPass(ID3D12GraphicsCommandList* cmdList) = 0;
};

ID3DBlob* LoadCSO(const std::string& filepath);

