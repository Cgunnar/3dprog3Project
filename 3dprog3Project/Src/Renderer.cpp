#include "pch.h"
#include "Renderer.h"

#pragma comment( lib, "d3d12.lib")
#pragma comment( lib, "d3dcompiler.lib")
#pragma comment( lib, "dxgi.lib")

Renderer::Renderer()
{
	IDXGIFactory6* factory6;
	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory6), reinterpret_cast<void**>(&factory6));
	assert(SUCCEEDED(hr));
}

Renderer::~Renderer()
{
}
