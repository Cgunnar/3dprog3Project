#include "pch.h"
#include "PostProcessingPass.h"

PostProcessingPass::PostProcessingPass(ID3D12Device* device, int framesInFlight)
	: m_device(device)
{
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;

#ifdef _DEBUG
	vsBlob = LoadCSO("Shaders/compiled/Debug/VertexShader.cso");
	psBlob = LoadCSO("Shaders/compiled/Debug/PixelShader.cso");
#else
	vsBlob = LoadCSO("Shaders/compiled/Release/VertexShader.cso");
	psBlob = LoadCSO("Shaders/compiled/Release/PixelShader.cso");
#endif // _DEBUG
}

PostProcessingPass::~PostProcessingPass()
{

}

void PostProcessingPass::RunRenderPass(ID3D12GraphicsCommandList* cmdList, int frameIndex)
{

	//cmdList->DrawInstanced(6, 1, 0, 0);
}

std::string PostProcessingPass::Name() const
{
	return "PostProcessingPass";
}
