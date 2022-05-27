#include "pch.h"
#include "TestRenderPass.h"
#include "rfEntity.hpp"
#include "CommonComponents.h"
#include "AssetManager.h"


TestRenderPass::TestRenderPass(RenderingSettings settings, ID3D12Device* device, DXGI_FORMAT renderTargetFormat)
	: RenderPass(settings), m_device(device), m_rtFormat(renderTargetFormat)
{
	m_constantBuffers.resize(m_numThreads);

	for (auto& t : m_constantBuffers)
	{
		t.resize(m_settings.numberOfFramesInFlight);
		for (auto& cbManager : t)
			cbManager = new ConstantBufferManager(device, 100000, 64);
	}
	

	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;

#ifdef _DEBUG
	vsBlob = LoadCSO("Shaders/compiled/Debug/VertexShader.cso");
	psBlob = LoadCSO("Shaders/compiled/Debug/PixelShader.cso");
#else
	vsBlob = LoadCSO("Shaders/compiled/Release/VertexShader.cso");
	psBlob = LoadCSO("Shaders/compiled/Release/PixelShader.cso");
#endif // _DEBUG

	std::array<D3D12_DESCRIPTOR_RANGE, m_numDescriptorsInRootTable0> vsDescriptorRanges;
	D3D12_DESCRIPTOR_RANGE descriptorRange;
	descriptorRange.NumDescriptors = 1;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.RegisterSpace = 0;

	//worldMatrix CB
	descriptorRange.BaseShaderRegister = 1;
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	vsDescriptorRanges[0] = descriptorRange;

	std::array<D3D12_ROOT_PARAMETER, 5> rootParameters;
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = m_numDescriptorsInRootTable0;
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

	//ib
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[3].Descriptor.ShaderRegister = 1;
	rootParameters[3].Descriptor.RegisterSpace = 0;
	//vb
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[4].Descriptor.ShaderRegister = 0;
	rootParameters[4].Descriptor.RegisterSpace = 0;

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
		if(errorBlob) errorBlob->Release();
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

TestRenderPass::~TestRenderPass()
{
	for (auto& t : m_constantBuffers)
	{
		for (auto& cbManager : t)
			delete cbManager;
	}
	
	m_pipelineState->Release();
	m_rootSignature->Release();
}

RenderPassRequirements TestRenderPass::GetRequirements()
{

	int perThreadSize = static_cast<int>(rfe::EntityReg::ViewEntities<MeshComp, MaterialComp, TransformComp>().size()) / m_numThreads;
	perThreadSize += rfe::EntityReg::ViewEntities<MeshComp, MaterialComp, TransformComp>().size() % m_numThreads;
	RenderPassRequirements req;
	req.cmdListCount = m_numThreads;
	req.descriptorHandleSize = m_numDescriptorsInRootTable0 * perThreadSize;
	req.numDescriptorHandles = m_numThreads;
	return req;
}

void TestRenderPass::Start(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{

}

static void Draw(int id, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorHandle& descHandle,
	std::vector<rfe::Entity> entitiesToDraw, FrameResource& frameResource,
	ConstantBufferManager* cbManager, int frameIndex, ID3D12RootSignature* rootSignature,
	ID3D12PipelineState* pipelineState, D3D12_GPU_VIRTUAL_ADDRESS camera);

void TestRenderPass::RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex)
{
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


	//fix a better way of allocating memory
	std::vector<rfe::Entity> entities = rfe::EntityReg::ViewEntities<MeshComp, MaterialComp, TransformComp>();

	std::vector<std::future<void>> asyncJobs;
	int segmentSize = static_cast<int>(entities.size()) / m_numThreads;
	for (int i = 0; i < m_numThreads; i++)
	{
		int rest = 0;
		if (i == m_numThreads - 1)
			rest = entities.size() % m_numThreads;

		std::vector<rfe::Entity> entitiesPerThread(entities.begin() + i * segmentSize, entities.begin() + (i + 1) * segmentSize + rest);

		asyncJobs.push_back(std::async(std::launch::async, Draw, i, m_device, cmdLists[i], std::ref(descriptorHandles[i]), entitiesPerThread, std::ref(frameResource),
			m_constantBuffers[i][frameIndex], frameIndex, m_rootSignature, m_pipelineState, m_constantBuffers[0][frameIndex]->GetGPUVirtualAddress(cameraCB)));
	}
	for (auto& j : asyncJobs)
		j.wait();
}

static void Draw(int id, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, DescriptorHandle& descHandle,
	std::vector<rfe::Entity> entitiesToDraw, FrameResource& frameResource,
	ConstantBufferManager* cbManager, int frameIndex, ID3D12RootSignature* rootSignature,
	ID3D12PipelineState* pipelineState, D3D12_GPU_VIRTUAL_ADDRESS camera)
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


	cmdList->SetGraphicsRootSignature(rootSignature);
	cmdList->SetPipelineState(pipelineState);

	cmdList->SetGraphicsRootConstantBufferView(2, camera);

	const AssetManager& am = AssetManager::Get();

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

		if (!vb.valid || !ib.valid || !colorBuffer.valid) continue;

		device->CopyDescriptorsSimple(1, currentCpuHandle, cbManager->GetAllDescriptors()[worldMatrixCB], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		currentCpuHandle.ptr += visBaseDescHandle.incrementSize;

		cmdList->SetGraphicsRootDescriptorTable(0, visBaseDescHandle.gpuHandle);
		visBaseDescHandle.gpuHandle.ptr += TestRenderPass::m_numDescriptorsInRootTable0 * visBaseDescHandle.incrementSize;
		cmdList->SetGraphicsRootConstantBufferView(1, colorBuffer.resource->GetGPUVirtualAddress());
		cmdList->SetGraphicsRootShaderResourceView(3, ib.resource->GetGPUVirtualAddress());
		cmdList->SetGraphicsRootShaderResourceView(4, vb.resource->GetGPUVirtualAddress());
		
		cmdList->DrawInstanced(ib.elementCount, 1, 0, 0);
	}
}

bool TestRenderPass::OnRenderingSettingsChange(RenderingSettings settings, ID3D12Device* device)
{
	if (m_settings.numberOfFramesInFlight != settings.numberOfFramesInFlight)
		return false;
	m_settings = settings;
	return true;// no need to recreate this class
	this->~TestRenderPass();
	new(this) TestRenderPass(settings, device, m_rtFormat);
}

std::string TestRenderPass::Name() const
{
	return "TestRenderPass";
}



//void TestRenderPass::RunRenderPass(std::vector<ID3D12GraphicsCommandList*> cmdLists, std::vector<DescriptorHandle> descriptorHandles, FrameResource& frameResource, int frameIndex)
//{
//	ID3D12GraphicsCommandList* cmdList = cmdLists.front();
//	auto [width, height] = frameResource.GetResolution();
//	D3D12_VIEWPORT viewport = { 0, 0, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
//	cmdList->RSSetViewports(1, &viewport);
//	D3D12_RECT scissorRect = { 0, 0, static_cast<long>(width), static_cast<long>(height) };
//	cmdList->RSSetScissorRects(1, &scissorRect);
//
//	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	float clearColor[] = { 0.2f, 0.0f, 0.0f, 0.0f };
//
//	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = frameResource.GetRtvCpuHandle();
//	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = frameResource.GetDsvCpuHandle();
//	cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
//	cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
//	cmdList->OMSetRenderTargets(1, &rtvHandle, true, &dsvHandle);
//
//	cmdList->SetGraphicsRootSignature(m_rootSignature);
//	cmdList->SetPipelineState(m_pipelineState);
//
//	m_constantBuffers[0][frameIndex]->Clear();
//
//	auto camera = rfe::EntityReg::ViewEntities<CameraComp>().front();
//	struct CameraCBData
//	{
//		rfm::Matrix proj;
//		rfm::Matrix view;
//		rfm::Matrix viewProj;
//		rfm::Vector3 pos;
//	} cameraCBData;
//	cameraCBData.proj = camera.GetComponent<CameraComp>()->projectionMatrix;
//	cameraCBData.view = rfm::inverse(camera.GetComponent<TransformComp>()->transform);
//	cameraCBData.viewProj = cameraCBData.proj * cameraCBData.view;
//	cameraCBData.pos = camera.GetComponent<TransformComp>()->transform.getTranslation();
//
//	UINT cameraCB = m_constantBuffers[0][frameIndex]->PushConstantBuffer();
//	m_constantBuffers[0][frameIndex]->UpdateConstantBuffer(cameraCB, &cameraCBData, sizeof(cameraCBData));
//
//	DescriptorHandle visBaseDescHandle = descriptorHandles.front();
//	D3D12_CPU_DESCRIPTOR_HANDLE currentCpuHandle = visBaseDescHandle.cpuHandle;
//
//	const AssetManager& am = AssetManager::Get();
//	//fix a better way of allocating memory
//	std::vector<rfe::Entity> entities = rfe::EntityReg::ViewEntities<MeshComp, MaterialComp, TransformComp>();
//	for (auto& entity : entities)
//	{
//		auto mesh = am.GetMesh(entity.GetComponent<MeshComp>()->meshID);
//		auto material = am.GetMaterial(entity.GetComponent<MaterialComp>()->materialID);
//		auto& transform = entity.GetComponent<TransformComp>()->transform;
//
//		UINT worldMatrixCB = m_constantBuffers[0][frameIndex]->PushConstantBuffer();
//		m_constantBuffers[0][frameIndex]->UpdateConstantBuffer(worldMatrixCB, &transform, 64);
//
//		GPUAsset vb = mesh.vertexBuffer;
//		GPUAsset ib = mesh.indexBuffer;
//		GPUAsset colorBuffer = material.constantBuffer;
//
//		if (!vb.valid || !ib.valid || !colorBuffer.valid) continue;
//
//		m_device->CopyDescriptorsSimple(1, currentCpuHandle, am.GetHeapDescriptors()[vb.descIndex], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//		currentCpuHandle.ptr += visBaseDescHandle.incrementSize;
//		m_device->CopyDescriptorsSimple(1, currentCpuHandle, am.GetHeapDescriptors()[ib.descIndex], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//		currentCpuHandle.ptr += visBaseDescHandle.incrementSize;
//		m_device->CopyDescriptorsSimple(1, currentCpuHandle, m_constantBuffers[0][frameIndex]->GetAllDescriptors()[worldMatrixCB], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//		currentCpuHandle.ptr += visBaseDescHandle.incrementSize;
//
//		cmdList->SetGraphicsRootDescriptorTable(0, visBaseDescHandle.gpuHandle);
//		visBaseDescHandle.gpuHandle.ptr += m_numDescriptorsInRootTable0 * visBaseDescHandle.incrementSize;
//		cmdList->SetGraphicsRootConstantBufferView(1, colorBuffer.resource->GetGPUVirtualAddress());
//		cmdList->SetGraphicsRootConstantBufferView(2, m_constantBuffers[0][frameIndex]->GetGPUVirtualAddress(cameraCB));
//		cmdList->DrawInstanced(ib.elementCount, 1, 0, 0);
//	}
//}