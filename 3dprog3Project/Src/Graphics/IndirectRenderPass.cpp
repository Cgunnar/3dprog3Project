#include "pch.h"
#include "IndirectRenderPass.h"
#include "rfEntity.hpp"
#include "CommonComponents.h"
#include "AssetManager.h"

IndirectRenderPass::IndirectRenderPass(ID3D12Device* device, int framesInFlight)
	: m_device(device), m_framesInFlight(framesInFlight)
{
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* csBlob = nullptr;

#ifdef _DEBUG
	vsBlob = LoadCSO("Shaders/compiled/Debug/VS_IndirectRenderPass.cso");
	psBlob = LoadCSO("Shaders/compiled/Debug/PS_IndirectRenderPass.cso");
	csBlob = LoadCSO("Shaders/compiled/Debug/CS_IndirectRenderPass.cso");
#else
	vsBlob = LoadCSO("Shaders/compiled/Release/VS_IndirectRenderPass.cso");
	psBlob = LoadCSO("Shaders/compiled/Release/PS_IndirectRenderPass.cso");
	csBlob = LoadCSO("Shaders/compiled/Release/CS_IndirectRenderPass.cso");
#endif // _DEBUG

	SetUpRenderPipeline(vsBlob, psBlob);
	SetUpComputePipeline(csBlob);
}

IndirectRenderPass::~IndirectRenderPass()
{
	m_pipelineState->Release();
	m_rootSignature->Release();
	m_pipelineStateCompute->Release();
	m_rootSignatureCompute->Release();
}

RenderPassRequirements IndirectRenderPass::GetRequirements()
{
	RenderPassRequirements req;
	req.cmdListCount = 2;
	req.descriptorHandleSize = rfe::EntityReg::ViewEntities<MeshComp, MaterialComp, TransformComp>().size();
	req.numDescriptorHandles = 1;
	return req;
}

void IndirectRenderPass::RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex)
{
	//cmdLists cant run compute work, add compute list

	DescriptorHandle descHandle = descriptorHandles.front();
	

	std::vector<rfe::Entity> entities = rfe::EntityReg::ViewEntities<MeshComp, MaterialComp, TransformComp>();
	if (entities.empty()) return;

	uint64_t meshID = entities.front().GetComponent<MeshComp>()->meshID;
	const auto& mesh = AssetManager::Get().GetMesh(meshID);
	const GPUAsset& vb = mesh.vertexBuffer;
	const GPUAsset& ib = mesh.indexBuffer;

	auto currentCpuHandle = descHandle.cpuHandle;
	m_device->CopyDescriptorsSimple(2, currentCpuHandle, AssetManager::Get().GetHeapDescriptors()[vb.descIndex], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	currentCpuHandle.ptr += 2 * descHandle.incrementSize;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = frameResource.GetRtvCpuHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = frameResource.GetDsvCpuHandle();
	float clearColor[] = { 0.2f, 0.0f, 0.0f, 0.0f };
	cmdLists.front()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	cmdLists.front()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	cmdLists.front()->SetGraphicsRootSignature(m_rootSignature);
	cmdLists.front()->SetPipelineState(m_pipelineState);

	cmdLists.front()->SetGraphicsRoot32BitConstant(meshIndexRP, 0, 0);
	cmdLists.front()->SetGraphicsRootDescriptorTable(vbBindlessRP, descHandle.gpuHandle);
	cmdLists.front()->SetGraphicsRootDescriptorTable(ibBindlessRP, descHandle[1].gpuHandle);
}

void IndirectRenderPass::RecreateOnResolutionChange(ID3D12Device* device, int framesInFlight, UINT width, UINT height)
{
	return;
}

std::string IndirectRenderPass::Name() const
{
	return "IndirectRenderPass";
}

void IndirectRenderPass::SetUpRenderPipeline(ID3DBlob* vs, ID3DBlob* ps)
{

	std::array<D3D12_ROOT_PARAMETER, 3> rootParameters;
	//root constant
	rootParameters[meshIndexRP].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[meshIndexRP].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[meshIndexRP].Constants.RegisterSpace = 0;
	rootParameters[meshIndexRP].Constants.ShaderRegister = 0;
	rootParameters[meshIndexRP].Constants.Num32BitValues = 1;

	//bindless vb
	D3D12_DESCRIPTOR_RANGE vbRange;
	vbRange.BaseShaderRegister = 0;
	vbRange.RegisterSpace = 1;
	vbRange.NumDescriptors = -1;
	vbRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	vbRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	rootParameters[vbBindlessRP].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[vbBindlessRP].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[vbBindlessRP].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[vbBindlessRP].DescriptorTable.pDescriptorRanges = &vbRange;

	D3D12_DESCRIPTOR_RANGE ibRange;
	ibRange.BaseShaderRegister = 0;
	ibRange.RegisterSpace = 2;
	ibRange.NumDescriptors = -1;
	ibRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	ibRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	rootParameters[ibBindlessRP].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[ibBindlessRP].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[ibBindlessRP].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[ibBindlessRP].DescriptorTable.pDescriptorRanges = &ibRange;

	D3D12_RASTERIZER_DESC rasterState;
	rasterState.FillMode = D3D12_FILL_MODE_SOLID;
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

	hr = m_device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), reinterpret_cast<void**>(&m_rootSignature));
	assert(SUCCEEDED(hr));
	rootSignatureBlob->Release();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
	pipelineStateDesc.pRootSignature = m_rootSignature;
	pipelineStateDesc.VS.BytecodeLength = vs->GetBufferSize();
	pipelineStateDesc.VS.pShaderBytecode = vs->GetBufferPointer();
	pipelineStateDesc.PS.BytecodeLength = ps->GetBufferSize();
	pipelineStateDesc.PS.pShaderBytecode = ps->GetBufferPointer();
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

	hr = m_device->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), reinterpret_cast<void**>(&m_pipelineState));
	assert(SUCCEEDED(hr));
}

void IndirectRenderPass::SetUpComputePipeline(ID3DBlob* cs)
{
	std::array<D3D12_ROOT_PARAMETER, 1> rootParameters;
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].Constants.RegisterSpace = 0;
	rootParameters[0].Constants.ShaderRegister = 0;
	rootParameters[0].Constants.Num32BitValues = 1;

	D3D12_ROOT_SIGNATURE_DESC rootSignDesc;
	rootSignDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	rootSignDesc.NumParameters = rootParameters.size();
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

	hr = m_device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), reinterpret_cast<void**>(&m_rootSignatureCompute));
	assert(SUCCEEDED(hr));
	rootSignatureBlob->Release();

	D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineStateDesc{};
	pipelineStateDesc.CS.BytecodeLength = cs->GetBufferSize();
	pipelineStateDesc.CS.pShaderBytecode = cs->GetBufferPointer();
	pipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	pipelineStateDesc.pRootSignature = m_rootSignatureCompute;

	hr = m_device->CreateComputePipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), reinterpret_cast<void**>(&m_pipelineStateCompute));
	assert(SUCCEEDED(hr));
}