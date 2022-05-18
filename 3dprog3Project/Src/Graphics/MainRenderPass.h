#pragma once
#include "RenderPass.h"
#include "StructuredBuffer.h"
#include "ConstantBufferManager.h"
#include "CommonComponents.h"

class MainRenderPass : public RenderPass
{
public:
	MainRenderPass(ID3D12Device* device, int framesInFlight, DXGI_FORMAT renderTargetFormat, int numThreads);
	~MainRenderPass();
	RenderPassRequirements GetRequirements() override;
	void RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex) override;
	void RecreateOnResolutionChange(ID3D12Device* device, int framesInFlight, UINT width, UINT height) override;
	std::string Name() const override;
	static constexpr UINT numDescriptorsInRootTable0 = 2; //per draw call vertexshader
	static constexpr UINT numDescriptorsInRootTable5 = 1; //bindless transforms vertexshader
	static constexpr UINT numDescriptorsInRootTable4 = 1; //bindless pixelshader
	static constexpr UINT numDescriptorsInRootTable3 = 1; //per frame pixelshader
	static constexpr UINT numDescriptorsInRootTable1 = 1; //bindless material pixelshader
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

