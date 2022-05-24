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

	D3D12_DESCRIPTOR_RANGE descriptorRange;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	descriptorRange.RegisterSpace = 2;
	descriptorRange.BaseShaderRegister = 0;

	std::array<D3D12_DESCRIPTOR_RANGE, numDescriptorsInRootTable6> table6;
	std::array<D3D12_DESCRIPTOR_RANGE, numDescriptorsInRootTable7> table7;
	descriptorRange.NumDescriptors = AssetManager::maxNumIndexBuffers;
	table6[0] = descriptorRange;
	descriptorRange.RegisterSpace = 4;
	descriptorRange.NumDescriptors = AssetManager::maxNumVertexBuffers;
	table7[0] = descriptorRange;

	////worldMatrix CB
	//descriptorRange.BaseShaderRegister = 1;
	//descriptorRange.RegisterSpace = 0;
	//descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	//vsDescriptorRanges[2] = descriptorRange;

	std::array<D3D12_DESCRIPTOR_RANGE, numDescriptorsInRootTable3> tableSlot3;
	descriptorRange.NumDescriptors = 1;
	descriptorRange.RegisterSpace = 0;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	//dynamicPointLights buffer
	descriptorRange.BaseShaderRegister = 0;
	tableSlot3[0] = descriptorRange;

	//ray traicing acceleration structure
	descriptorRange.BaseShaderRegister = 1;
	tableSlot3[1] = descriptorRange;

	//data for indexOffset and materialIndex
	descriptorRange.BaseShaderRegister = 2;
	tableSlot3[2] = descriptorRange;

	//bindless
	std::array<D3D12_DESCRIPTOR_RANGE, numDescriptorsInRootTable4> psPerDrawCallDescriptors;
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

	std::array<D3D12_ROOT_PARAMETER, 9> rootParameters;
	/*rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = numDescriptorsInRootTable0;
	rootParameters[0].DescriptorTable.pDescriptorRanges = vsDescriptorRanges.data();*/

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[0].Constants.ShaderRegister = 1;
	rootParameters[0].Constants.RegisterSpace = 0;
	rootParameters[0].Constants.Num32BitValues = 1;
	//ib
	rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[6].DescriptorTable.NumDescriptorRanges = numDescriptorsInRootTable6;
	rootParameters[6].DescriptorTable.pDescriptorRanges = table6.data();
	//vb
	rootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[7].DescriptorTable.NumDescriptorRanges = numDescriptorsInRootTable7;
	rootParameters[7].DescriptorTable.pDescriptorRanges = table7.data();

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

	rootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[8].Constants.ShaderRegister = 2;
	rootParameters[8].Constants.RegisterSpace = 3;
	rootParameters[8].Constants.Num32BitValues = 5;


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
	staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
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
	req.descriptorHandleSize = (numDescriptorsInRootTable0 + numDescriptorsInRootTable5) * 10 * renderCount + numDescriptorsInRootTable3
		+ numDescriptorsInRootTable6 + numDescriptorsInRootTable7;
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

struct RenderUnit
{
	rfm::Matrix worldMatrix;
	uint64_t subMeshID;
	uint64_t meshID;
	uint32_t indexBufferDescriptorIndex;
	uint32_t vertexBufferDescriptorIndex;
	uint32_t indexStart;
	uint32_t indexCount;
	uint32_t vertexStart;
	int32_t materialDescriptorIndex;
};

static void Draw(int id, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorHandle& descHandle,
	std::vector<rfe::Entity> entitiesToDraw, FrameResource& frameResource,
	ConstantBufferManager* cbManager, int frameIndex);


static void Draw2(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorHandle& descHandle,
	std::vector<RenderUnit>& renderUnits, FrameResource& frameResource,
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
	descriptorHandle.cpuHandle.ptr += descriptorHandle.incrementSize;
	m_device->CopyDescriptorsSimple(1, descriptorHandle.cpuHandle, m_accelerationStructures[frameIndex]->GetAccelerationStructureCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHandle.cpuHandle.ptr += descriptorHandle.incrementSize;
	m_device->CopyDescriptorsSimple(1, descriptorHandle.cpuHandle, m_accelerationStructures[frameIndex]->GetInstanceMetaDataCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHandle.cpuHandle.ptr += descriptorHandle.incrementSize;
	auto& cmdList = cmdLists.front();
	cmdList->SetGraphicsRootSignature(m_rootSignature);
	cmdList->SetPipelineState(m_pipelineState);
	cmdList->SetGraphicsRootConstantBufferView(2, m_constantBuffers[frameIndex]->GetGPUVirtualAddress(cameraCB));
	cmdList->SetGraphicsRootDescriptorTable(3, descriptorHandle.gpuHandle);
	descriptorHandle.gpuHandle.ptr += descriptorHandle.incrementSize * numDescriptorsInRootTable3;
	descriptorHandle.index += numDescriptorsInRootTable3;


	//fix a better way of allocating memory
	std::vector<rfe::Entity> entities = rfe::EntityReg::ViewEntities<MeshComp, MaterialComp, TransformComp>();

	std::vector<RenderUnit> renderUnits;
	renderUnits.reserve(entities.size() * 2);
	const auto& am = AssetManager::Get();
	for (auto& e : entities)
	{
		const auto& meshComp = e.GetComponent<MeshComp>();
		const auto& materialComp = e.GetComponent<MaterialComp>();
		const auto& meshAsset = am.GetMesh(meshComp->meshID);
		const auto& materialAsset = am.GetMaterial(materialComp->materialID);
		if (meshAsset.subMeshes)
		{
			for (auto& subMesh : meshAsset.subMeshes->subMeshes)
			{
				RenderUnit ru;
				ru.worldMatrix = e.GetComponent<TransformComp>()->transform;
				ru.materialDescriptorIndex = am.GetMaterial(subMesh.materialID).constantBuffer.descIndex;
				ru.indexBufferDescriptorIndex = meshAsset.indexBuffer.descIndex;
				ru.vertexBufferDescriptorIndex = meshAsset.vertexBuffer.descIndex;
				ru.indexStart = subMesh.indexStart;
				ru.indexCount = subMesh.indexCount;
				ru.vertexStart = subMesh.vertexStart;
				ru.subMeshID = subMesh.subMeshID;
				ru.meshID = meshComp->meshID;
				renderUnits.push_back(std::move(ru));
			}
		}
		else
		{
			RenderUnit ru;
			ru.worldMatrix = e.GetComponent<TransformComp>()->transform;
			ru.materialDescriptorIndex = materialAsset.constantBuffer.descIndex;
			ru.indexBufferDescriptorIndex = meshAsset.indexBuffer.descIndex;
			ru.vertexBufferDescriptorIndex = meshAsset.vertexBuffer.descIndex;
			ru.indexStart = 0;
			ru.indexCount = meshAsset.indexBuffer.elementCount;
			ru.vertexStart = 0;
			ru.subMeshID = 0;
			ru.meshID = meshComp->meshID;
			renderUnits.push_back(std::move(ru));
		}
	}

	Draw2(m_device, cmdList, descriptorHandle, renderUnits, frameResource,
		m_constantBuffers[frameIndex], frameIndex);

	//std::sort(entities.begin(), entities.end(), [](rfe::Entity& a, rfe::Entity& b) {
	//	const auto& meshA = a.GetComponent<MeshComp>();
	//	const auto& meshB = b.GetComponent<MeshComp>();
	//	/*if (meshA->meshID == meshB->meshID)
	//	{
	//		return meshA->indexCount < meshB->indexCount;
	//	}*/
	//	return meshA->meshID < meshB->meshID;
	//	});

	//Draw(0, m_device, cmdList, descriptorHandle, entities, frameResource,
	//	m_constantBuffers[frameIndex], frameIndex);
}

static void Draw2(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorHandle& descHandle,
	std::vector<RenderUnit>& renderUnits, FrameResource& frameResource,
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
	cmdList->SetGraphicsRootDescriptorTable(6, am.GetBindlessIndexBufferStart().gpuHandle);
	cmdList->SetGraphicsRootDescriptorTable(7, am.GetBindlessVertexBufferStart().gpuHandle);


	std::sort(renderUnits.begin(), renderUnits.end(), [](RenderUnit& a, RenderUnit& b) {
		if (a.meshID == b.meshID)
		{
			return a.subMeshID < b.subMeshID;
		}
		return a.meshID < b.meshID;
		});

	int counter = 0;
	for (auto& ru : renderUnits)
	{
		UINT worldMatrixCB = cbManager->PushConstantBuffer();
		struct PerInstanceData
		{
			rfm::Matrix worldMatrix;
			int materialDescriptorIndex;
		};
		PerInstanceData instanceData{ ru.worldMatrix, ru.materialDescriptorIndex };
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
	uint64_t nextSubMeshID = 0;
	int numEntitiesToDraw = renderUnits.size();

	for (int i = 0; i < numEntitiesToDraw; i++)
	{
		auto& ru = renderUnits[i];
		
		uint64_t meshID = i == 0 ? ru.meshID  : nextMeshID;
		uint64_t subMeshID = i == 0 ? ru.subMeshID  : nextSubMeshID;
		if (i < numEntitiesToDraw - 1)
		{
			nextMeshID = renderUnits[i + 1].meshID;
			nextSubMeshID = renderUnits[i + 1].subMeshID;
		}

		if (meshID != nextMeshID || subMeshID != nextSubMeshID || i == numEntitiesToDraw - 1)
		{
			cmdList->SetGraphicsRoot32BitConstant(8, counter, 0);
			cmdList->SetGraphicsRoot32BitConstant(8, ru.indexBufferDescriptorIndex, 1);
			cmdList->SetGraphicsRoot32BitConstant(8, ru.vertexBufferDescriptorIndex, 2);
			cmdList->SetGraphicsRoot32BitConstant(8, ru.indexStart, 3);
			cmdList->SetGraphicsRoot32BitConstant(8, ru.vertexStart, 4);
			cmdList->DrawInstanced(ru.indexCount, numInstances, 0, 0);
			counter += numInstances;
			numInstances = 1;
		}
		else
		{
			numInstances++;
		}
	}
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
	cmdList->SetGraphicsRootDescriptorTable(6, am.GetBindlessIndexBufferStart().gpuHandle);
	cmdList->SetGraphicsRootDescriptorTable(7, am.GetBindlessVertexBufferStart().gpuHandle);

	

	int counter = 0;
	for (auto& entity : entitiesToDraw)
	{
		const auto& transform = entity.GetComponent<TransformComp>()->transform;
		uint64_t matID = entity.GetComponent<MaterialComp>()->materialID;
		const auto& meshComp = entity.GetComponent<MeshComp>();
		const auto& meshAsset = am.GetMesh(meshComp->meshID);
		if (meshAsset.subMeshes)
		{
			for (auto& subMesh : meshAsset.subMeshes->subMeshes)
			{
				int matDescIndex = am.GetMaterial(subMesh.materialID).constantBuffer.descIndex;
				UINT worldMatrixCB = cbManager->PushConstantBuffer();
				struct PerInstanceData
				{
					rfm::Matrix worldMatrix;
					int materialDescriptorIndex;
				};
				PerInstanceData instanceData{ transform, matDescIndex };
				cbManager->UpdateConstantBuffer(worldMatrixCB, &instanceData, sizeof(PerInstanceData));

				//can add local transforms for submeshes later
				device->CopyDescriptorsSimple(1, descHandle[counter].cpuHandle, cbManager->GetAllDescriptors()[worldMatrixCB], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				counter++;
			}
		}
		else
		{
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
		/*uint64_t matID = meshComp->materialID;
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
		counter++;*/
	}

	cmdList->SetGraphicsRootDescriptorTable(5, descHandle.gpuHandle);
	DescriptorHandle visBaseDescHandle = descHandle[counter];
	D3D12_CPU_DESCRIPTOR_HANDLE currentCpuHandle = visBaseDescHandle.cpuHandle;
	counter = 0;
	int numInstances = 1;
	uint64_t nextMeshID = 0;
	uint64_t nextMeshIndexStart = 0;
	uint64_t nextMeshIndexCount = 0;
	uint64_t nextMeshVertexStart = 0;
	uint64_t nextMeshVertexCount = 0;
	int numEntitiesToDraw = entitiesToDraw.size();

	for (int i = 0; i < numEntitiesToDraw; i++)
	{
		auto& entity = entitiesToDraw[i];

		const auto& meshComp = entity.GetComponent<MeshComp>();
		uint64_t meshID = i == 0 ? meshComp->meshID : nextMeshID;
		if (i < numEntitiesToDraw - 1)
		{
			const auto& nextMeshComp = entitiesToDraw[i + 1].GetComponent<MeshComp>();
			nextMeshIndexCount = nextMeshComp->indexCount;
			nextMeshIndexStart = nextMeshComp->indexStart;
			nextMeshVertexCount = nextMeshComp->vertexCount;
			nextMeshVertexStart = nextMeshComp->vertexStart;
			nextMeshID = nextMeshComp->meshID;
		}

		/*bool differentSubMesh = meshComp->indexCount != nextMeshIndexCount || meshComp->indexStart != nextMeshIndexStart
			|| meshComp->vertexCount != nextMeshVertexCount || meshComp->vertexStart != nextMeshVertexStart;*/

		if (meshID != nextMeshID || /*differentSubMesh ||*/ i == numEntitiesToDraw - 1)
		{
			const auto& mesh = am.GetMesh(meshID);

			if (mesh.subMeshes)
			{
				for (auto& subMesh : mesh.subMeshes->subMeshes)
				{
					const GPUAsset& vb = mesh.vertexBuffer;
					const GPUAsset& ib = mesh.indexBuffer;

					if (!vb.valid || !ib.valid) assert(false);

					cmdList->SetGraphicsRoot32BitConstant(8, counter, 0);
					cmdList->SetGraphicsRoot32BitConstant(8, ib.descIndex, 1);
					cmdList->SetGraphicsRoot32BitConstant(8, vb.descIndex, 2);
					cmdList->SetGraphicsRoot32BitConstant(8, subMesh.indexStart, 3);
					cmdList->SetGraphicsRoot32BitConstant(8, subMesh.vertexStart, 4);
					cmdList->DrawInstanced(subMesh.indexCount, numInstances, 0, 0);
					counter += numInstances;
				}
				numInstances = 1;
			}
			else
			{

				const GPUAsset& vb = mesh.vertexBuffer;
				const GPUAsset& ib = mesh.indexBuffer;

				if (!vb.valid || !ib.valid) assert(false);

				cmdList->SetGraphicsRoot32BitConstant(8, counter, 0);
				cmdList->SetGraphicsRoot32BitConstant(8, ib.descIndex, 1);
				cmdList->SetGraphicsRoot32BitConstant(8, vb.descIndex, 2);
				cmdList->SetGraphicsRoot32BitConstant(8, meshComp->indexStart, 3);
				cmdList->SetGraphicsRoot32BitConstant(8, meshComp->vertexStart, 4);
				cmdList->DrawInstanced(meshComp->indexCount != 0 ? meshComp->indexCount : ib.elementCount, numInstances, 0, 0);
				counter += numInstances;
				numInstances = 1;
			}
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