#include "pch.h"
#include "MainRenderPass.h"
#include "rfEntity.hpp"
#include "CommonComponents.h"
#include "AssetManager.h"


MainRenderPass::MainRenderPass(ID3D12Device* device, int framesInFlight)
	: m_device(device)
{
	m_constantBuffers.resize(m_numThreads);

	for (auto& t : m_constantBuffers)
	{
		t.resize(framesInFlight);
		for (auto& cbManager : t)
			cbManager = new ConstantBufferManager(device, 100000, 64);
	}


	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;

#ifdef _DEBUG
	vsBlob = LoadCSO("Shaders/compiled/Debug/VS_MainRenderPass.cso");
	psBlob = LoadCSO("Shaders/compiled/Debug/PS_MainRenderPass.cso");
#else
	vsBlob = LoadCSO("Shaders/compiled/Release/VS_MainRenderPass.cso");
	psBlob = LoadCSO("Shaders/compiled/Release/PS_MainRenderPass.cso");
#endif // _DEBUG

	std::array<D3D12_DESCRIPTOR_RANGE, numDescriptorsInRootTable0> vsDescriptorRanges;
	D3D12_DESCRIPTOR_RANGE descriptorRange;
	descriptorRange.NumDescriptors = 1;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.RegisterSpace = 0;

	descriptorRange.BaseShaderRegister = 0;
	vsDescriptorRanges[0] = descriptorRange;
	descriptorRange.BaseShaderRegister = 1;
	vsDescriptorRanges[1] = descriptorRange;

	//worldMatrix CB
	descriptorRange.BaseShaderRegister = 1;
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	vsDescriptorRanges[2] = descriptorRange;

	std::array<D3D12_DESCRIPTOR_RANGE, numDescriptorsInRootTable3> tableSlot3;
	descriptorRange.NumDescriptors = 1;
	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.RegisterSpace = 0;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	//dynamicPointLights buffer
	tableSlot3[0] = descriptorRange;

	//bindless
	std::array<D3D12_DESCRIPTOR_RANGE, numDescriptorsInRootTable0> psPerDrawCallDescriptors;
	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.RegisterSpace = 1;
	descriptorRange.NumDescriptors = AssetManager::maxNumAlbedoTextures;
	psPerDrawCallDescriptors[0] = descriptorRange; //this need to be the last in the table def

	std::array<D3D12_ROOT_PARAMETER, 5> rootParameters;
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = numDescriptorsInRootTable0;
	rootParameters[0].DescriptorTable.pDescriptorRanges = vsDescriptorRanges.data();

	//material CB
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].Descriptor.ShaderRegister = 1;
	rootParameters[1].Descriptor.RegisterSpace = 0;

	//camera CB
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[2].Descriptor.ShaderRegister = 0;
	rootParameters[2].Descriptor.RegisterSpace = 0;

	//table with pointlightSRV
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].DescriptorTable.NumDescriptorRanges = numDescriptorsInRootTable3;
	rootParameters[3].DescriptorTable.pDescriptorRanges = tableSlot3.data();

	//pixelshader texture table
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[4].DescriptorTable.NumDescriptorRanges = numDescriptorsInRootTable4;
	rootParameters[4].DescriptorTable.pDescriptorRanges = psPerDrawCallDescriptors.data();

	D3D12_RASTERIZER_DESC rasterState;
	rasterState.FillMode = D3D12_FILL_MODE_SOLID;
	//rasterState.CullMode = D3D12_CULL_MODE_NONE;
	rasterState.CullMode = D3D12_CULL_MODE_BACK;
	rasterState.FrontCounterClockwise = false;
	rasterState.DepthBias = 0;
	rasterState.DepthBiasClamp = 0.0f;
	rasterState.SlopeScaledDepthBias = 0.0f;
	rasterState.DepthClipEnable = true;
	rasterState.MultisampleEnable = false;
	rasterState.AntialiasedLineEnable = false;
	rasterState.ForcedSampleCount = 0;
	rasterState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	D3D12_STATIC_SAMPLER_DESC staticSampler;
	staticSampler.Filter = D3D12_FILTER_ANISOTROPIC;
	staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSampler.MipLODBias = 0.0f;
	staticSampler.MaxAnisotropy = 16;
	staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	staticSampler.MinLOD = 0;
	staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
	staticSampler.ShaderRegister = 0;
	staticSampler.RegisterSpace = 0;
	staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_RENDER_TARGET_BLEND_DESC rtvBlendDesc;
	rtvBlendDesc.BlendEnable = false;
	rtvBlendDesc.LogicOpEnable = false;
	rtvBlendDesc.SrcBlend = D3D12_BLEND_ONE;
	rtvBlendDesc.DestBlend = D3D12_BLEND_ZERO;
	rtvBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	rtvBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	rtvBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	rtvBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rtvBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	rtvBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_DEPTH_STENCIL_DESC dssDesc;
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	dssDesc.StencilEnable = false;
	dssDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	dssDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	dssDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	dssDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	dssDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	dssDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	dssDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	dssDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	dssDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	dssDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	D3D12_STREAM_OUTPUT_DESC streamOutPutDesc;
	streamOutPutDesc.pSODeclaration = nullptr;
	streamOutPutDesc.NumEntries = 0;
	streamOutPutDesc.pBufferStrides = nullptr;
	streamOutPutDesc.NumStrides = 0;
	streamOutPutDesc.RasterizedStream = 0;

	D3D12_ROOT_SIGNATURE_DESC rootSignDesc;
	rootSignDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	//rtx 2070 mobile does only have shader model 6.5
	//rootSignDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
	rootSignDesc.NumParameters = rootParameters.size();
	rootSignDesc.pParameters = rootParameters.data();
	rootSignDesc.NumStaticSamplers = 1;
	rootSignDesc.pStaticSamplers = &staticSampler;

	ID3DBlob* rootSignatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&rootSignDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSignatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			std::cout << static_cast<char*>(errorBlob->GetBufferPointer()) << std::endl;
			errorBlob->Release();
		}
		assert(SUCCEEDED(hr));
	}

	hr = device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), reinterpret_cast<void**>(&m_rootSignature));
	assert(SUCCEEDED(hr));
	rootSignatureBlob->Release();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
	pipelineStateDesc.pRootSignature = m_rootSignature;
	pipelineStateDesc.VS.BytecodeLength = vsBlob->GetBufferSize();
	pipelineStateDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	pipelineStateDesc.PS.BytecodeLength = psBlob->GetBufferSize();
	pipelineStateDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.RasterizerState = rasterState;
	pipelineStateDesc.NumRenderTargets = 1u;
	pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipelineStateDesc.BlendState.RenderTarget[0] = rtvBlendDesc;
	pipelineStateDesc.BlendState.AlphaToCoverageEnable = false;
	pipelineStateDesc.BlendState.IndependentBlendEnable = false;
	pipelineStateDesc.SampleDesc.Count = 1u;
	pipelineStateDesc.SampleDesc.Quality = 0u;
	pipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateDesc.DepthStencilState = dssDesc;
	pipelineStateDesc.StreamOutput = streamOutPutDesc;
	pipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	hr = device->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), reinterpret_cast<void**>(&m_pipelineState));
	assert(SUCCEEDED(hr));


	m_dynamicPointLightBuffer.resize(framesInFlight);
	for (auto& lightBuffer : m_dynamicPointLightBuffer)
	{
		lightBuffer = std::make_unique<StructuredBuffer<PointLight>>(device, 1000, true, true);
	}
}

MainRenderPass::~MainRenderPass()
{
	for (auto& t : m_constantBuffers)
	{
		for (auto& cbManager : t)
			delete cbManager;
	}

	m_pipelineState->Release();
	m_rootSignature->Release();
}

RenderPassRequirements MainRenderPass::GetRequirements()
{

	int perThreadSize = rfe::EntityReg::ViewEntities<MeshComp, MaterialComp, TransformComp>().size() / m_numThreads;
	perThreadSize += rfe::EntityReg::ViewEntities<MeshComp, MaterialComp, TransformComp>().size() % m_numThreads;
	RenderPassRequirements req;
	req.cmdListCount = m_numThreads;
	req.descriptorHandleSize = (numDescriptorsInRootTable0 + numDescriptorsInRootTable4) * perThreadSize + numDescriptorsInRootTable3;
	req.numDescriptorHandles = m_numThreads;
	return req;
}

static void Draw(int id, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorHandle& descHandle,
	std::vector<rfe::Entity> entitiesToDraw, FrameResource& frameResource,
	ConstantBufferManager* cbManager, int frameIndex);

void MainRenderPass::RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex)
{
	UpdateDynamicLights(frameIndex);

	for (int i = 0; i < m_numThreads; i++)
	{
		m_constantBuffers[i][frameIndex]->Clear();
	}

	auto camera = rfe::EntityReg::ViewEntities<CameraComp>().front();
	struct CameraCBData
	{
		rfm::Matrix proj;
		rfm::Matrix view;
		rfm::Matrix viewProj;
		rfm::Vector3 pos;
	} cameraCBData;
	cameraCBData.proj = camera.GetComponent<CameraComp>()->projectionMatrix;
	cameraCBData.view = rfm::inverse(camera.GetComponent<TransformComp>()->transform);
	cameraCBData.viewProj = cameraCBData.proj * cameraCBData.view;
	cameraCBData.pos = camera.GetComponent<TransformComp>()->transform.getTranslation();

	UINT cameraCB = m_constantBuffers[0][frameIndex]->PushConstantBuffer();
	m_constantBuffers[0][frameIndex]->UpdateConstantBuffer(cameraCB, &cameraCBData, sizeof(cameraCBData));

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = frameResource.GetRtvCpuHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = frameResource.GetDsvCpuHandle();
	float clearColor[] = { 0.2f, 0.0f, 0.0f, 0.0f };
	cmdLists.front()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	cmdLists.front()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//we use one of the descriptors that we requaseted for thread0, to bind the lights
	m_device->CopyDescriptorsSimple(1, descriptorHandles.front().cpuHandle, m_dynamicPointLightBuffer[frameIndex]->CpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHandles.front().cpuHandle.ptr += descriptorHandles.front().incrementSize;

	for (auto& cmdList : cmdLists)
	{
		cmdList->SetGraphicsRootSignature(m_rootSignature);
		cmdList->SetPipelineState(m_pipelineState);
		cmdList->SetGraphicsRootConstantBufferView(2, m_constantBuffers[0][frameIndex]->GetGPUVirtualAddress(cameraCB));
		cmdList->SetGraphicsRootDescriptorTable(3, descriptorHandles.front().gpuHandle);
	}
	descriptorHandles.front().gpuHandle.ptr += descriptorHandles.front().incrementSize;


	//fix a better way of allocating memory
	std::vector<rfe::Entity> entities = rfe::EntityReg::ViewEntities<MeshComp, MaterialComp, TransformComp>();
	rfm::Vector3 cameraPos = cameraCBData.pos;
	std::sort(entities.begin(), entities.end(), [&cameraPos](rfe::Entity& a, rfe::Entity& b) {
		rfm::Vector3 distA = a.GetComponent<TransformComp>()->transform.getTranslation() - cameraPos;
		rfm::Vector3 distB = b.GetComponent<TransformComp>()->transform.getTranslation() - cameraPos;
		return rfm::dot(distA, distA) < rfm::dot(distB, distB);
		});

	if (m_numThreads == 1)
	{
		Draw(0, m_device, cmdLists[0], std::ref(descriptorHandles[0]), entities, std::ref(frameResource),
			m_constantBuffers[0][frameIndex], frameIndex);
	}
	else
	{
		std::vector<std::future<void>> asyncJobs;
		int segmentSize = entities.size() / m_numThreads;
		for (int i = 0; i < m_numThreads; i++)
		{
			int rest = 0;
			if (i == m_numThreads - 1)
				rest = entities.size() % m_numThreads;

			std::vector<rfe::Entity> entitiesPerThread(entities.begin() + i * segmentSize, entities.begin() + (i + 1) * segmentSize + rest);

			asyncJobs.push_back(std::async(std::launch::async, Draw, i, m_device, cmdLists[i], std::ref(descriptorHandles[i]), entitiesPerThread, std::ref(frameResource),
				m_constantBuffers[i][frameIndex], frameIndex));
		}
		for (auto& j : asyncJobs)
			j.wait();
	}
}

static void Draw(int id, ID3D12Device * device, ID3D12GraphicsCommandList * cmdList, DescriptorHandle & descHandle,
	std::vector<rfe::Entity> entitiesToDraw, FrameResource& frameResource,
	ConstantBufferManager* cbManager, int frameIndex)
{
	auto [width, height] = frameResource.GetResolution();
	D3D12_VIEWPORT viewport = { 0, 0, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
	cmdList->RSSetViewports(1, &viewport);
	D3D12_RECT scissorRect = { 0, 0, static_cast<long>(width), static_cast<long>(height) };
	cmdList->RSSetScissorRects(1, &scissorRect);

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = frameResource.GetRtvCpuHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = frameResource.GetDsvCpuHandle();
	cmdList->OMSetRenderTargets(1, &rtvHandle, true, &dsvHandle);

	const AssetManager& am = AssetManager::Get();
	cmdList->SetGraphicsRootDescriptorTable(4, am.GetBindlessAlbedoTexturesStart().gpuHandle);


	DescriptorHandle visBaseDescHandle = descHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE currentCpuHandle = visBaseDescHandle.cpuHandle;

	for (auto& entity : entitiesToDraw)
	{
		const auto& mesh = am.GetMesh(entity.GetComponent<MeshComp>()->meshID);
		const auto& material = am.GetMaterial(entity.GetComponent<MaterialComp>()->materialID);
		const auto& transform = entity.GetComponent<TransformComp>()->transform;

		UINT worldMatrixCB = cbManager->PushConstantBuffer();
		cbManager->UpdateConstantBuffer(worldMatrixCB, &transform, 64);

		const GPUAsset& vb = mesh.vertexBuffer;
		const GPUAsset& ib = mesh.indexBuffer;
		const GPUAsset& colorBuffer = material.constantBuffer;
		const GPUAsset& albedoTexture = material.albedoTexture;

		if (!vb.valid || !ib.valid || !colorBuffer.valid) continue;

		device->CopyDescriptorsSimple(1, currentCpuHandle, am.GetHeapDescriptors()[vb.descIndex], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		currentCpuHandle.ptr += visBaseDescHandle.incrementSize;
		device->CopyDescriptorsSimple(1, currentCpuHandle, am.GetHeapDescriptors()[ib.descIndex], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		currentCpuHandle.ptr += visBaseDescHandle.incrementSize;
		device->CopyDescriptorsSimple(1, currentCpuHandle, cbManager->GetAllDescriptors()[worldMatrixCB], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		currentCpuHandle.ptr += visBaseDescHandle.incrementSize;

		cmdList->SetGraphicsRootDescriptorTable(0, visBaseDescHandle.gpuHandle);
		visBaseDescHandle.gpuHandle.ptr += MainRenderPass::numDescriptorsInRootTable0 * visBaseDescHandle.incrementSize;
		cmdList->SetGraphicsRootConstantBufferView(1, colorBuffer.resource->GetGPUVirtualAddress());

		cmdList->DrawInstanced(ib.elementCount, 1, 0, 0);
	}
}

void MainRenderPass::RecreateOnResolutionChange(ID3D12Device* device, int framesInFlight, UINT width, UINT height)
{
	return; // no need to recreate this class
	this->~MainRenderPass();
	new(this) MainRenderPass(device, framesInFlight);
}

std::string MainRenderPass::Name() const
{
	return "MainRenderPass";
}

void MainRenderPass::UpdateDynamicLights(int frameIndex)
{
	const auto& lightComps = rfe::EntityReg::GetComponentArray<PointLightComp>();

	std::vector<PointLight> lights;
	lights.reserve(lightComps.size());
	for (const auto& c : lightComps)
		if (c.lit) lights.push_back(c.pointLight);

	//this could be done better, we uppdate all lights even if they have not changed
	if (!lights.empty())
	{
		m_dynamicPointLightBuffer[frameIndex]->Update(lights.data(), lights.size());
	}
}