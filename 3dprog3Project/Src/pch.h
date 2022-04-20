#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#define NOMINMAX
#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>

#include <assert.h>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <filesystem>

#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>
#include <memory>

#include <string>
#include <vector>
#include <array>
#include <queue>
#include <stack>
#include <map>
#include <unordered_map>
#include <tuple>
#include <variant>
#include <optional>
#include <functional>

#include <mutex>
#include <thread>
#include <atomic>
#include <semaphore>
#include <future>

#include "UtilityFunctions.h"
#include "RenderingTypes.h"