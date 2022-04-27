#pragma once

struct MemoryInfo
{
	UINT64 adapterMemory;
	UINT64 applicationMemoryUsage;
	UINT64 memoryBudgetFromOS;
};

struct RenderingSettings
{
	UINT renderWidth = 1920;
	UINT renderHeight = 1080;
	UINT numberOfBackbuffers = 3;
	UINT numberOfFramesInFlight = 2;
	bool hasChanged = true;
};