#pragma once

#include <cstdint>
#include <functional>

namespace Sky::RHI
{

using SurfaceFactoryFn = std::function<void*(void* backendInstance)>;

struct SwapchainCreateInfo
{
  SurfaceFactoryFn surfaceFactory;
  uint32_t width = 0;
  uint32_t height = 0;
};

struct Extent2D { uint32_t width = 0; uint32_t height = 0; };

}