#pragma once

// C++ stdlib — тяжёлые headers которые нужны почти везде
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// Vulkan
#include <vulkan/vulkan.h>

// spdlog (тяжёлый с fmt)
#include <spdlog/spdlog.h>

// Our core infrastructure
#include "Core/Core.h"
#include "Core/Log.h"
