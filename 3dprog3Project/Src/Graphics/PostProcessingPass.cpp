#include "pch.h"
#include "PostProcessingPass.h"
#include "Window.h"

PostProcessingPass::PostProcessingPass(ID3D12Device* device, int framesInFlight)
	: m_device(device)
{
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;

#ifdef _DEBUG
	vsBlob = LoadCSO("Shaders/compiled/Debug/VS_quad.cso");
	psBlob = LoadCSO("Shaders/compiled/Debug/PS_postprocessing.cso");
#else
	vsBlob = LoadCSO("Shaders/compiled/Release/VS_quad.cso");
	psBlob = LoadCSO("Shaders/compiled/Release/PS_postprocessing.cso");
#endif // _DEBUG

	D3D12_DESCRIPTOR_RANGE descRange;
	descRange.NumDescriptors = 1;
	descRange.BaseShaderRegister = 0;
	descRange.RegisterSpace = 0;
	descRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	std::array<D3D12_ROOT_PARAMETER, 1> rootParameters;
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descRange;

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

	D3D12_STATIC_SAMPLER_DESC staticSampler;
	staticSampler.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
	staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSampler.MipLODBias = 0.0f;
	staticSampler.MaxAnisotropy = 1;
	staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	staticSampler.MinLOD = 0;
	staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
	staticSampler.ShaderRegister = 0;
	staticSampler.RegisterSpace = 0;
	staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

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
		errorBlob->Release();
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
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipelineStateDesc.BlendState.RenderTarget[0] = rtvBlendDesc;
	pipelineStateDesc.BlendState.AlphaToCoverageEnable = false;
	pipelineStateDesc.BlendState.IndependentBlendEnable = false;
	pipelineStateDesc.SampleDesc.Count = 1;
	pipelineStateDesc.SampleDesc.Quality = 0;
	pipelineStateDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	pipelineStateDesc.DepthStencilState = dssDesc;
	pipelineStateDesc.StreamOutput = streamOutPutDesc;
	pipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	hr = device->CreateGraphicsPipelineState(&pipelineStateDesc, __uuidof(ID3D12PipelineState), reinterpret_cast<void**>(&m_pipelineState));
	assert(SUCCEEDED(hr));
}

PostProcessingPass::~PostProcessingPass()
{
	m_rootSignature->Release();
	m_pipelineState->Release();
}

RenderPassRequirements PostProcessingPass::GetRequirements()
{
	RenderPassRequirements req;
	req.cmdListCount = 1;
	req.numDescriptorHandles = 1;
	req.descriptorHandleSize = 1;
	return req;
}

void PostProcessingPass::RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex)
{
	ID3D12GraphicsCommandList* cmdList = cmdLists.front();
	auto [width, height] = Window::GetWidthAndHeight();
	D3D12_VIEWPORT viewport = { 0, 0, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
	cmdList->RSSetViewports(1, &viewport);
	D3D12_RECT scissorRect = { 0, 0, static_cast<long>(width), static_cast<long>(height) };
	cmdList->RSSetScissorRects(1, &scissorRect);

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = frameResource.GetBackBufferCpuHandle();
	cmdList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);

	cmdList->SetGraphicsRootSignature(m_rootSignature);
	cmdList->SetPipelineState(m_pipelineState);

	D3D12_RESOURCE_BARRIER transitionToSRVBarrier;
	transitionToSRVBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	transitionToSRVBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	transitionToSRVBarrier.Transition.pResource = frameResource.renderTarget;
	transitionToSRVBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	transitionToSRVBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	transitionToSRVBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	cmdList->ResourceBarrier(1, &transitionToSRVBarrier);

	m_device->CopyDescriptorsSimple(1, descriptorHandles.front().cpuHandle, frameResource.GetRtSrvCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//UINT srvTableIndex = frameResource.GetHeapDescriptor().Push(frameResource.GetRtSrvCpuHandle(), m_device);
	cmdList->SetGraphicsRootDescriptorTable(0, descriptorHandles.front().gpuHandle);

	cmdList->DrawInstanced(6, 1, 0, 0);

	D3D12_RESOURCE_BARRIER transitionToRTVBarrier;
	transitionToRTVBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	transitionToRTVBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	transitionToRTVBarrier.Transition.pResource = frameResource.renderTarget;
	transitionToRTVBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	transitionToRTVBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	transitionToRTVBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	cmdList->ResourceBarrier(1, &transitionToRTVBarrier);
}

void PostProcessingPass::RecreateOnResolutionChange(ID3D12Device* device, int framesInFlight, UINT width, UINT height)
{
	//this class does not need the resolution but the solution is to good to not be used
	this->~PostProcessingPass();
	new(this) PostProcessingPass(device, framesInFlight);
}

std::string PostProcessingPass::Name() const
{
	return "PostProcessingPass";
}
