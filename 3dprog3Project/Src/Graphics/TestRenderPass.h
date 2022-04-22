#pragma once
#include "RenderPass.h"
class TestRenderPass : public RenderPass
{
public:
	TestRenderPass() = default;
	~TestRenderPass() = default;

	void RunRenderPass(ID3D12GraphicsCommandList* cmdList) override;
};

