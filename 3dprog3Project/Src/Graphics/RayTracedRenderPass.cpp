#include "pch.h"
#include "RayTracedRenderPass.h"
#include "AssetManager.h"

RayTracedRenderPass::RayTracedRenderPass(ID3D12Device* device, int framesInFlight, DXGI_FORMAT renderTargetFormat)
	: m_device(device), m_rtFormat(renderTargetFormat), m_numberOfFramesInFlight(framesInFlight)
{
	m_constantBuffers.resize(framesInFlight);

	for (auto& cbManager : m_constantBuffers)
	{
		cbManager = new ConstantBufferManager(device, 100000, 256);
	}

	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;

#ifdef _DEBUG
	vsBlob = LoadCSO("Shaders/compiled/Debug/VS_RayTracedRenderPass.cso");
	psBlob = LoadCSO("Shaders/compiled/Debug/PS_RayTracedRenderPass.cso");
#else
	vsBlob = LoadCSO("Shaders/compiled/Release/VS_RayTracedRenderPass.cso");
	psBlob = LoadCSO("Shaders/compiled/Release/PS_RayTracedRenderPass.cso");
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

	////worldMatrix CB
	//descriptorRange.BaseShaderRegister = 1;
	//descriptorRange.RegisterSpace = 0;
	//descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	//vsDescriptorRanges[2] = descriptorRange;

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

	//bindless transforms
	std::array<D3D12_DESCRIPTOR_RANGE, numDescriptorsInRootTable5> tableSlot5;
	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.RegisterSpace = 1;
	descriptorRange.NumDescriptors = -1;
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	tableSlot5[0] = descriptorRange;

	//bindless materialCB
	std::array<D3D12_DESCRIPTOR_RANGE, numDescriptorsInRootTable1> tableSlot1;
	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.RegisterSpace = 2;
	descriptorRange.NumDescriptors = AssetManager::maxNumMaterials;
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	tableSlot1[0] = descriptorRange;

	std::array<D3D12_ROOT_PARAMETER, 7> rootParameters;
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = numDescriptorsInRootTable0;
	rootParameters[0].DescriptorTable.pDescriptorRanges = vsDescriptorRanges.data();

	//material CB
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = numDescriptorsInRootTable1;
	rootParameters[1].DescriptorTable.pDescriptorRanges = tableSlot1.data();

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

	//vertex transform table
	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[5].DescriptorTable.NumDescriptorRanges = numDescriptorsInRootTable5;
	rootParameters[5].DescriptorTable.pDescriptorRanges = tableSlot5.data();

	rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[6].Constants.ShaderRegister = 2;
	rootParameters[6].Constants.RegisterSpace = 3;
	rootParameters[6].Constants.Num32BitValues = 1;


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
	pipelineStateDesc.RTVFormats[0] = renderTargetFormat;
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

RayTracedRenderPass::~RayTracedRenderPass()
{
	for (auto& cbManager : m_constantBuffers)
		delete cbManager;

	m_pipelineState->Release();
	m_rootSignature->Release();
}

RenderPassRequirements RayTracedRenderPass::GetRequirements()
{
	int renderCount = rfe::EntityReg::ViewEntities<MeshComp, MaterialComp, TransformComp>().size();
	RenderPassRequirements req;
	req.cmdListCount = 1;
	req.descriptorHandleSize = (numDescriptorsInRootTable0 + numDescriptorsInRootTable5) * renderCount + numDescriptorsInRootTable3;
	req.numDescriptorHandles = 1;
	return req;
}

void RayTracedRenderPass::Start(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	ID3D12Device5* device5 = nullptr;
	HRESULT hr = device->QueryInterface(__uuidof(ID3D12Device5), reinterpret_cast<void**>(&device5));
	assert(SUCCEEDED(hr));
	ID3D12GraphicsCommandList4* cmdList4 = nullptr;
	hr = cmdList->QueryInterface(__uuidof(ID3D12GraphicsCommandList4), reinterpret_cast<void**>(&cmdList4));
	assert(SUCCEEDED(hr));
	m_accelerationStructures.resize(m_numberOfFramesInFlight);
	for (auto& a : m_accelerationStructures)
		a = std::make_unique<AccelerationStructure>(device5, cmdList4);
	device5->Release();
	cmdList4->Release();
}

static void Draw(int id, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorHandle& descHandle,
	std::vector<rfe::Entity> entitiesToDraw, FrameResource& frameResource,
	ConstantBufferManager* cbManager, int frameIndex);

void RayTracedRenderPass::RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex)
{
	UpdateDynamicLights(frameIndex);
	m_accelerationStructures[frameIndex]->UpdateTopLevel(m_device, cmdLists.front());

	m_constantBuffers[frameIndex]->Clear();

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

	UINT cameraCB = m_constantBuffers[frameIndex]->PushConstantBuffer();
	m_constantBuffers[frameIndex]->UpdateConstantBuffer(cameraCB, &cameraCBData, sizeof(cameraCBData));

	auto& descriptorHandle = descriptorHandles.front();
	m_device->CopyDescriptorsSimple(1, descriptorHandle.cpuHandle, m_dynamicPointLightBuffer[frameIndex]->CpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHandles.front().cpuHandle.ptr += descriptorHandle.incrementSize;
	auto& cmdList = cmdLists.front();
	cmdList->SetGraphicsRootSignature(m_rootSignature);
	cmdList->SetPipelineState(m_pipelineState);
	cmdList->SetGraphicsRootConstantBufferView(2, m_constantBuffers[frameIndex]->GetGPUVirtualAddress(cameraCB));
	cmdList->SetGraphicsRootDescriptorTable(3, descriptorHandle.gpuHandle);
	descriptorHandle.gpuHandle.ptr += descriptorHandle.incrementSize;
	descriptorHandle.index++;


	//fix a better way of allocating memory
	std::vector<rfe::Entity> entities = rfe::EntityReg::ViewEntities<MeshComp, MaterialComp, TransformComp>();
	rfm::Vector3 cameraPos = cameraCBData.pos;

	std::sort(entities.begin(), entities.end(), [](rfe::Entity& a, rfe::Entity& b) {
		return a.GetComponent<MeshComp>()->meshID < b.GetComponent<MeshComp>()->meshID;
		});

	Draw(0, m_device, cmdList, descriptorHandle, entities, frameResource,
		m_constantBuffers[frameIndex], frameIndex);
}

static void Draw(int id, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorHandle& descHandle,
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
	cmdList->SetGraphicsRootDescriptorTable(1, am.GetBindlessMaterialStart().gpuHandle);
	cmdList->SetGraphicsRootDescriptorTable(4, am.GetBindlessAlbedoTexturesStart().gpuHandle);

	int counter = 0;
	for (auto& entity : entitiesToDraw)
	{
		const auto& transform = entity.GetComponent<TransformComp>()->transform;
		uint64_t matID = entity.GetComponent<MaterialComp>()->materialID;
		int matDescIndex = am.GetMaterial(matID).constantBuffer.descIndex;
		UINT worldMatrixCB = cbManager->PushConstantBuffer();
		struct PerInstanceData
		{
			rfm::Matrix worldMatrix;
			int materialDescriptorIndex;
		};
		PerInstanceData instanceData{ transform, matDescIndex };
		cbManager->UpdateConstantBuffer(worldMatrixCB, &instanceData, sizeof(PerInstanceData));

		device->CopyDescriptorsSimple(1, descHandle[counter].cpuHandle, cbManager->GetAllDescriptors()[worldMatrixCB], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		counter++;
	}

	cmdList->SetGraphicsRootDescriptorTable(5, descHandle.gpuHandle);
	DescriptorHandle visBaseDescHandle = descHandle[counter];
	D3D12_CPU_DESCRIPTOR_HANDLE currentCpuHandle = visBaseDescHandle.cpuHandle;
	counter = 0;
	int numInstances = 1;
	uint64_t nextMeshID = 0;
	int numEntitiesToDraw = entitiesToDraw.size();

	for (int i = 0; i < numEntitiesToDraw; i++)
	{
		auto& entity = entitiesToDraw[i];

		uint64_t meshID = i == 0 ? entity.GetComponent<MeshComp>()->meshID : nextMeshID;
		if (i < numEntitiesToDraw - 1)
		{
			nextMeshID = entitiesToDraw[i + 1].GetComponent<MeshComp>()->meshID;
		}

		if (meshID != nextMeshID || i == numEntitiesToDraw - 1)
		{
			const auto& mesh = am.GetMesh(meshID);

			const GPUAsset& vb = mesh.vertexBuffer;
			const GPUAsset& ib = mesh.indexBuffer;

			if (!vb.valid || !ib.valid) assert(false);

			device->CopyDescriptorsSimple(2, currentCpuHandle, am.GetHeapDescriptors()[vb.descIndex], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			currentCpuHandle.ptr += 2 * visBaseDescHandle.incrementSize;

			cmdList->SetGraphicsRootDescriptorTable(0, visBaseDescHandle.gpuHandle);
			visBaseDescHandle.gpuHandle.ptr += RayTracedRenderPass::numDescriptorsInRootTable0 * visBaseDescHandle.incrementSize;
			cmdList->SetGraphicsRoot32BitConstant(6, counter, 0);
			cmdList->DrawInstanced(ib.elementCount, numInstances, 0, 0);
			counter += numInstances;
			numInstances = 1;
		}
		else
		{
			numInstances++;
		}
	}
}

void RayTracedRenderPass::RecreateOnResolutionChange(ID3D12Device* device, int framesInFlight, UINT width, UINT height)
{
	return;
}

std::string RayTracedRenderPass::Name() const
{
	return "RayTracedRenderPass";
}

void RayTracedRenderPass::UpdateDynamicLights(int frameIndex)
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