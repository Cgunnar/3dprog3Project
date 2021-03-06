#pragma once
#include "RimfrostMath.hpp"

extern int g_drawCallsPerFrame;

struct MemoryInfo
{
	UINT64 adapterMemory;
	UINT64 applicationMemoryUsage;
	UINT64 memoryBudgetFromOS;
};
enum class FullscreenState
{
	windowed,
	borderLess,
	fullscreen //this is not full-screen exclusive mode
};
struct RenderingSettings
{
	FullscreenState fullscreemState = FullscreenState::windowed;
	UINT renderWidth = 1280;
	UINT renderHeight = 720;
	UINT numberOfBackbuffers = 2;
	UINT numberOfFramesInFlight = 2;
	UINT numberOfBounces = 0;
	bool shadows = false;
	bool vsync = false;
	bool zPrePass = false;
	bool instancing = true;
};

struct RenderUnit
{
	rfm::Matrix worldMatrix;
	uint32_t indexBufferDescriptorIndex = 0;
	uint32_t vertexBufferDescriptorIndex = 0;
	uint32_t indexStart = 0;
	uint32_t indexCount = 0;
	uint32_t vertexStart = 0;
	int32_t materialDescriptorIndex = -1;
	int32_t vertexType = 0; // 1 == has tangents and bitangents
	int32_t extraInt = 0;
	uint64_t subMeshID = 0;
	uint64_t meshID = 0;
};