#include "pch.h"
#include "TestRenderPass.h"
#include "rfEntity.hpp"
#include "CommonComponents.h"
#include "AssetManager.h"


TestRenderPass::TestRenderPass(ID3D12Device* device) : m_device(device)
{
	m_heapDescriptor.Init(device);

	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;

#ifdef _DEBUG
	vsBlob = LoadCSO("Shaders/compiled/Debug/VertexShader.cso");
	psBlob = LoadCSO("Shaders/compiled/Debug/PixelShader.cso");
#else
	vsBlob = LoadCSO("Shaders/compiled/Release/VertexShader.cso");
	psBlob = LoadCSO("Shaders/compiled/Release/PixelShader.cso");
#endif // _DEBUG

	std::vector<D3D12_DESCRIPTOR_RANGE> vsDescriptorRanges;
	D3D12_DESCRIPTOR_RANGE descriptorRange;
	descriptorRange.NumDescriptors = 1;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.RegisterSpace = 0;
	
	descriptorRange.BaseShaderRegister = 0;
	vsDescriptorRanges.push_back(descriptorRange);
	descriptorRange.BaseShaderRegister = 1;
	vsDescriptorRanges.push_back(descriptorRange);

	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	vsDescriptorRanges.push_back(descriptorRange);

	std::array<D3D12_ROOT_PARAMETER, 2> rootParameters;
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = vsDescriptorRanges.size();
	rootParameters[0].DescriptorTable.pDescriptorRanges = vsDescriptorRanges.data();

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].Descriptor.ShaderRegister = 0;
	rootParameters[1].Descriptor.RegisterSpace = 0;

	D3D12_RASTERIZER_DESC rasterState;
	rasterState.FillMode = D3D12_FILL_MODE_SOLID;
	rasterState.CullMode = D3D12_CULL_MODE_NONE;
	//rasterState.CullMode = D3D12_CULL_MODE_BACK;
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
	rootSignDesc.NumParameters = rootParameters.size();
	rootSignDesc.pParameters = rootParameters.data();
	rootSignDesc.NumStaticSamplers = 0;
	rootSignDesc.pStaticSamplers = nullptr;

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

}

TestRenderPass::~TestRenderPass()
{
	m_pipelineState->Release();
	m_rootSignature->Release();
}

void TestRenderPass::RunRenderPass(ID3D12GraphicsCommandList* cmdList)
{
	m_heapDescriptor.Clear();
	auto heapDescriptor = m_heapDescriptor.Get();
	cmdList->SetDescriptorHeaps(1, &heapDescriptor);
	cmdList->SetGraphicsRootSignature(m_rootSignature);
	cmdList->SetPipelineState(m_pipelineState);

	const AssetManager& am = AssetManager::Get();
	//fix a better way of allocating memory
	std::vector<rfe::Entity> entities = rfe::EntityReg::ViewEntities<MeshComp, MaterialComp, TransformComp>();
	for (auto& entity : entities)
	{
		auto mesh = am.GetMesh(entity.GetComponent<MeshComp>()->meshID);
		auto material = am.GetMaterial(entity.GetComponent<MaterialComp>()->materialID);
		auto transform = entity.GetComponent<TransformComp>()->transform;

		GPUAsset vb = mesh.vertexBuffer;
		GPUAsset ib = mesh.indexBuffer;
		GPUAsset colorBuffer = material.constantBuffer;

		if (!vb.valid || !ib.valid || !colorBuffer.valid) continue;

		UINT tableSlot0 = m_heapDescriptor.Size();
		m_heapDescriptor.AddFromOther(am.GetHeapDescriptors(), vb.descIndex, 1, m_device);
		m_heapDescriptor.AddFromOther(am.GetHeapDescriptors(), ib.descIndex, 1, m_device);

		cmdList->SetGraphicsRootDescriptorTable(0, m_heapDescriptor.GetGPUHandle(tableSlot0));
		cmdList->SetGraphicsRootConstantBufferView(1, colorBuffer.resource->GetGPUVirtualAddress());
		cmdList->DrawInstanced(ib.elementCount, 1, 0, 0);
	}
}