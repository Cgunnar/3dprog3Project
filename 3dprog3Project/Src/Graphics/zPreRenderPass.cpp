#include "pch.h"
#include "zPreRenderPass.h"
#include "AssetManager.h"
#include "rfEntity.hpp"
#include "CommonComponents.h"

ZPreRenderPass::ZPreRenderPass(RenderingSettings settings, ID3D12Device* device, DXGI_FORMAT renderTargetFormat)
	: RenderPass(settings), m_device(device), m_rtFormat(renderTargetFormat)
{
	m_constantBuffers.resize(m_settings.numberOfFramesInFlight);

	for (auto& cbManager : m_constantBuffers)
	{
		cbManager = new ConstantBufferManager(device, 100000, 256);
	}

	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;

#ifdef _DEBUG
	vsBlob = LoadCSO("Shaders/compiled/Debug/VS_zPrePass.cso");
	psBlob = LoadCSO("Shaders/compiled/Debug/PS_empty.cso");
#else
	vsBlob = LoadCSO("Shaders/compiled/Release/VS_zPrePass.cso");
	psBlob = LoadCSO("Shaders/compiled/Release/PS_empty.cso");
#endif // _DEBUG




	D3D12_DESCRIPTOR_RANGE descriptorRange;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	descriptorRange.RegisterSpace = 2;
	descriptorRange.BaseShaderRegister = 0;

	std::array<D3D12_DESCRIPTOR_RANGE, 1> table1;
	std::array<D3D12_DESCRIPTOR_RANGE, 1> table2;
	std::array<D3D12_DESCRIPTOR_RANGE, 1> table3;
	std::array<D3D12_DESCRIPTOR_RANGE, 1> table5;
	descriptorRange.NumDescriptors = AssetManager::maxNumIndexBuffers;
	table1[0] = descriptorRange;
	descriptorRange.RegisterSpace = 4;
	descriptorRange.NumDescriptors = AssetManager::maxNumVertexBuffers;
	table2[0] = descriptorRange;
	descriptorRange.RegisterSpace = 5;
	table3[0] = descriptorRange;

	//bindless transforms
	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.RegisterSpace = 1;
	descriptorRange.NumDescriptors = -1;
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	table5[0] = descriptorRange;



	std::array<D3D12_ROOT_PARAMETER, 6> rootParameters;

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[0].Constants.ShaderRegister = 2;
	rootParameters[0].Constants.RegisterSpace = 3;
	rootParameters[0].Constants.Num32BitValues = 6;
	//ib
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[1].DescriptorTable.pDescriptorRanges = table1.data();
	//vb
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[2].DescriptorTable.pDescriptorRanges = table2.data();

	//vb for normal mapping, meshes have not separeted buffers for positions so this is needed
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[3].DescriptorTable.pDescriptorRanges = table3.data();

	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[4].Descriptor.ShaderRegister = 0;
	rootParameters[4].Descriptor.RegisterSpace = 0;

	//vertex transform table
	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[5].DescriptorTable.pDescriptorRanges = table5.data();

	



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
	rootSignDesc.NumParameters = static_cast<UINT>(rootParameters.size());
	rootSignDesc.pParameters = rootParameters.data();
	rootSignDesc.NumStaticSamplers = 0;
	rootSignDesc.pStaticSamplers = nullptr;

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
}

ZPreRenderPass::~ZPreRenderPass()
{
	for (auto& cbManager : m_constantBuffers)
		delete cbManager;

	m_pipelineState->Release();
	m_rootSignature->Release();
}

RenderPassRequirements ZPreRenderPass::GetRequirements()
{
	int renderCount = static_cast<int>(m_renderUnits.size());
	RenderPassRequirements req;
	req.cmdListCount = 1;
	req.descriptorHandleSize = renderCount;
	req.numDescriptorHandles = 1;
	return req;
}

void ZPreRenderPass::Start(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
}

void ZPreRenderPass::SubmitObjectsToRender(const std::vector<RenderUnit>& renderUnits)
{
	m_renderUnits = renderUnits;
	std::sort(m_renderUnits.begin(), m_renderUnits.end(), [](RenderUnit& a, RenderUnit& b) {
		if (a.meshID == b.meshID)
		{
			return a.subMeshID < b.subMeshID;
		}
		return a.meshID < b.meshID;
	});
}

static void Draw(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorHandle& descHandle,
	std::vector<RenderUnit>& renderUnits, FrameResource& frameResource,
	ConstantBufferManager* cbManager, int frameIndex, bool instancing);

void ZPreRenderPass::RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex)
{
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
	auto& cmdList = cmdLists.front();
	cmdList->SetGraphicsRootSignature(m_rootSignature);
	cmdList->SetPipelineState(m_pipelineState);
	

	const AssetManager& am = AssetManager::Get();
	cmdList->SetGraphicsRootDescriptorTable(1, am.GetBindlessIndexBufferStart().gpuHandle);
	cmdList->SetGraphicsRootDescriptorTable(2, am.GetBindlessVertexBufferStart().gpuHandle);
	cmdList->SetGraphicsRootDescriptorTable(3, am.GetBindlessVertexBufferStart().gpuHandle);
	cmdList->SetGraphicsRootConstantBufferView(4, m_constantBuffers[frameIndex]->GetGPUVirtualAddress(cameraCB));


	auto [width, height] = frameResource.GetResolution();
	D3D12_VIEWPORT viewport = { 0, 0, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
	cmdList->RSSetViewports(1, &viewport);
	D3D12_RECT scissorRect = { 0, 0, static_cast<long>(width), static_cast<long>(height) };
	cmdList->RSSetScissorRects(1, &scissorRect);

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = frameResource.GetDsvCpuHandle();
	cmdList->OMSetRenderTargets(0, nullptr, true, &dsvHandle);


	Draw(m_device, cmdList, descriptorHandle, m_renderUnits, frameResource,
		m_constantBuffers[frameIndex], frameIndex, m_settings.instancing);
}


static void Draw(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorHandle& descHandle,
	std::vector<RenderUnit>& renderUnits, FrameResource& frameResource,
	ConstantBufferManager* cbManager, int frameIndex, bool instancing)
{
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

	counter = 0;
	int numInstances = 1;
	int numEntitiesToDraw = static_cast<int>(renderUnits.size());

	for (int i = 0; i < numEntitiesToDraw; i++)
	{
		auto& ru = renderUnits[i];

		if (i == numEntitiesToDraw - 1 || !instancing ||
			ru.meshID != renderUnits[i + 1].meshID ||
			ru.subMeshID != renderUnits[i + 1].subMeshID)
		{
			cmdList->SetGraphicsRoot32BitConstant(0, counter, 0);
			cmdList->SetGraphicsRoot32BitConstant(0, ru.indexBufferDescriptorIndex, 1);
			cmdList->SetGraphicsRoot32BitConstant(0, ru.vertexBufferDescriptorIndex, 2);
			cmdList->SetGraphicsRoot32BitConstant(0, ru.indexStart, 3);
			cmdList->SetGraphicsRoot32BitConstant(0, ru.vertexStart, 4);
			cmdList->SetGraphicsRoot32BitConstant(0, ru.vertexType, 5);
			cmdList->DrawInstanced(ru.indexCount, numInstances, 0, 0);
			counter += numInstances;
			g_drawCallsPerFrame++;
			numInstances = 0;
		}
		numInstances++;
	}
}

bool ZPreRenderPass::OnRenderingSettingsChange(RenderingSettings settings, ID3D12Device* device)
{
	if (m_settings.numberOfFramesInFlight != settings.numberOfFramesInFlight)
		return false;
	m_settings = settings;
	return true;
}

std::string ZPreRenderPass::Name() const
{
	return "ZPreRenderPass";
}
