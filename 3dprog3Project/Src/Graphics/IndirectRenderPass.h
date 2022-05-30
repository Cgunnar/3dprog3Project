#pragma once
#include "RenderPass.h"
#include "StructuredBuffer.h"
class IndirectRenderPass : public RenderPass
{
public:
	IndirectRenderPass(RenderingSettings settings, ID3D12Device* device, DXGI_FORMAT renderTargetFormat);
	~IndirectRenderPass();
	RenderPassRequirements GetRequirements() override;
	void Start(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) override;
	void SubmitObjectsToRender(const std::vector<RenderUnit>& renderUnits) override;
	void RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex) override;
	bool OnRenderingSettingsChange(RenderingSettings settings, ID3D12Device* device) override;
	std::string Name() const override;

private:
	constexpr static size_t meshIndexRP = 0;
	constexpr static size_t renderUnitBufferRP = 1;
	constexpr static size_t ibBindlessRP = 2;
	constexpr static size_t vbBindlessRP = 3;
	constexpr static size_t vbtBindlessRP = 4;

	DXGI_FORMAT m_rtFormat;
	int m_framesInFlight;
	ID3D12Device* m_device = nullptr;

	std::vector<RenderUnit> m_renderUnits;

	std::vector<StructuredBuffer<RenderUnit>*> m_gpuRenderUnitsBuffers;

	ID3D12RootSignature* m_rootSignature = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;

	ID3D12RootSignature* m_rootSignatureCompute = nullptr;
	ID3D12PipelineState* m_pipelineStateCompute = nullptr;

	void SetUpRenderPipeline(ID3DBlob* vs, ID3DBlob* ps);
	void SetUpComputePipeline(ID3DBlob* cs);
};

