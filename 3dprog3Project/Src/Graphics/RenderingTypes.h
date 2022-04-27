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
	UINT renderWidth = 1920;
	UINT renderHeight = 1080;
	UINT numberOfBackbuffers = 3;
	UINT numberOfFramesInFlight = 2;
	bool hasChanged = false;
};