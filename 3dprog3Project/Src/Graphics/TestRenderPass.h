#pragma once
#include "RenderPass.h"
#include "DescriptorVector.h"
#include "ConstantBufferManager.h"
class TestRenderPass : public RenderPass
{
public:
	TestRenderPass(ID3D12Device* device, int framesInFlight);
	~TestRenderPass();
	RenderPassRequirements GetRequirements() override;
	void RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex) override;
	void RecreateOnResolutionChange(ID3D12Device* device, int framesInFlight, UINT width, UINT height) override;
	std::string Name() const override;
	static constexpr UINT m_numDescriptorsInRootTable0 = 3;
private:
	ID3D12Device* m_device = nullptr;
	ID3D12RootSignature* m_rootSignature = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;
	int m_numThreads = 1;
	std::vector<std::vector<ConstantBufferManager*>> m_constantBuffers;
};

