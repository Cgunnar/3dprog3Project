#pragma once
#include "RenderPass.h"
#include "StructuredBuffer.h"
#include "ConstantBufferManager.h"
#include "CommonComponents.h"
#include "AccelerationStructure.h"

struct RenderUnit
{
	rfm::Matrix worldMatrix;
	uint64_t subMeshID = 0;
	uint64_t meshID = 0;
	uint32_t indexBufferDescriptorIndex = 0;
	uint32_t vertexBufferDescriptorIndex = 0;
	uint32_t indexStart = 0;
	uint32_t indexCount = 0;
	uint32_t vertexStart = 0;
	int32_t materialDescriptorIndex = -1;
};

class RayTracedRenderPass : public RenderPass
{
public:
	RayTracedRenderPass(ID3D12Device* device, int framesInFlight, DXGI_FORMAT renderTargetFormat);
	~RayTracedRenderPass();
	RenderPassRequirements GetRequirements() override;
	void Start(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) override;
	void RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex) override;
	void RecreateOnResolutionChange(ID3D12Device* device, int framesInFlight, UINT width, UINT height) override;
	std::string Name() const override;

	static constexpr UINT numDescriptorsInRootTable0 = 0; //per draw call vertexshader
	static constexpr UINT numDescriptorsInRootTable5 = 1; //bindless transforms vertexshader
	static constexpr UINT numDescriptorsInRootTable4 = 1; //bindless pixelshader
	static constexpr UINT numDescriptorsInRootTable3 = 3; //per frame pixelshader
	static constexpr UINT numDescriptorsInRootTable1 = 1; //bindless material pixelshader
	static constexpr UINT numDescriptorsInRootTable6 = 1; //bindless ib
	static constexpr UINT numDescriptorsInRootTable7 = 1; //bindless vb
private:
	ID3D12Device* m_device = nullptr;
	ID3D12RootSignature* m_rootSignature = nullptr;
	ID3D12PipelineState* m_pipelineState = nullptr;
	DXGI_FORMAT m_rtFormat;
	std::vector<ConstantBufferManager*> m_constantBuffers;

	std::vector<RenderUnit> m_renderUnits;

	std::vector<std::unique_ptr<StructuredBuffer<PointLight>>> m_dynamicPointLightBuffer;
	std::vector<std::unique_ptr<AccelerationStructure>> m_accelerationStructures;
	int m_numberOfFramesInFlight;
	int m_rayBounceCount = 2;
	int m_useShadows = 1;
	int FindObjectsToRender();
	[[nodiscard]] int UpdateDynamicLights(int frameIndex);
};

