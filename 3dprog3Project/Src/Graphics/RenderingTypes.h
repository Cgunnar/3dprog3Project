#pragma once

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
	UINT numberOfFramesInFlight = 1;
	bool hasChanged = false;
};