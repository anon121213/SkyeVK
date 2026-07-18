#pragma once

#include "Buffer.h"
#include "Handle.h"
#include "Swapchain.h"
#include "Types.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Sky::RHI
{

struct DeviceCreateInfo
{
  BackendType backend = BackendType::Auto;
  std::string appName = "SkyRenderApp";
  std::string engineName = "SkyEngine";
  std::vector<const char*> requiredInstanceExtensions;
  SurfaceFactoryFn surfaceFactory;
  uint32_t initialWindowWidth = 1280;
  uint32_t initialWindowHeight = 720;
  bool enableValidation = false;

  // Temporary for Phase R — full paths to triangle shaders.
  // Will be removed in Phase 1 when Frame Graph exposes pipeline/shader creation.
  std::string              vertShaderPath;
  std::string              fragShaderPath;
};

class Device
{
public:
  explicit Device(const DeviceCreateInfo&);
  ~Device() noexcept;

  Device(const Device &) = delete;
  Device& operator=(const Device &) = delete;

  void drawFrame();
  void waitIdle() const;

  [[nodiscard]] SwapchainHandle createSwapchain(const SwapchainCreateInfo& info);
  void destroySwapchain(SwapchainHandle handle) noexcept;
  [[nodiscard]] SwapchainHandle defaultSwapchain() const noexcept;

  [[nodiscard]] BufferHandle createBuffer(const BufferDesc& desc);
  void destroyBuffer(BufferHandle handle) noexcept;

  [[nodiscard]] void* mapBuffer(BufferHandle handle);
  void unmapBuffer(BufferHandle handle);

  void uploadBufferData(BufferHandle handle, const void* data, uint64_t size);

  struct Impl;

private:
  std::unique_ptr<Impl> m_Impl;
};

}
