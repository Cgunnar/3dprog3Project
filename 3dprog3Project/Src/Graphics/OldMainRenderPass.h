#pragma once
#include "RenderPass.h"
#include "StructuredBuffer.h"
#include "ConstantBufferManager.h"
#include "CommonComponents.h"

class OldMainRenderPass : public RenderPass
{
public:
	OldMainRenderPass(RenderingSettings settings, ID3D12Device* device, DXGI_FORMAT renderTargetFormat, int numThreads);
	~OldMainRenderPass();
	RenderPassRequirements GetRequirements() override;
	void Start(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) override;
	void SubmitObjectsToRender(const std::vector<RenderUnit>& renderUnits) override;
	void RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex) override;
	bool OnRenderingSettingsChange(RenderingSettings settings, ID3D12Device* device) override;
	std::string Name() const override;
	static constexpr UINT numDescriptorsInRootTable0 = 1; //per draw call vertexshader
	static constexpr UINT numDescriptorsInRootTable4 = 1; //bindless pixelshader
	static constexpr UINT numDescriptorsInRootTable3 = 1; //per frame pixelshader
private:
	ID3D12Device* m_device = nullptr;
	ID3D12RootSignature* m_rootSignature = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;
	int m_numThreads;
	DXGI_FORMAT m_rtFormat;
	std::vector<std::vector<ConstantBufferManager*>> m_constantBuffers;
	
	std::vector<std::unique_ptr<StructuredBuffer<PointLight>>> m_dynamicPointLightBuffer;

	void UpdateDynamicLights(int frameIndex);
};

