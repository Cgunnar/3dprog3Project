#pragma once
#include "RenderPass.h"
#include "DescriptorVector.h"
#include "ConstantBufferManager.h"
class TestRenderPass : public RenderPass
{
public:
	TestRenderPass(RenderingSettings settings, ID3D12Device* device, DXGI_FORMAT renderTargetFormat);
	~TestRenderPass();
	RenderPassRequirements GetRequirements() override;
	void Start(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) override;
	void RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex) override;
	bool OnRenderingSettingsChange(RenderingSettings settings, ID3D12Device* device) override;
	std::string Name() const override;
	static constexpr UINT m_numDescriptorsInRootTable0 = 1;
private:
	ID3D12Device* m_device = nullptr;
	ID3D12RootSignature* m_rootSignature = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;
	int m_numThreads = 12;
	std::vector<std::vector<ConstantBufferManager*>> m_constantBuffers;
	DXGI_FORMAT m_rtFormat;
};

