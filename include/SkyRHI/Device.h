#pragma once

#include "Buffer.h"
#include "Handle.h"
#include "Pipeline.h"
#include "Shader.h"
#include "Swapchain.h"
#include "Types.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Sky::RHI
{

class FrameGraph;   // forward decl — execute() takes it by reference; full def in DeviceImpl.cpp

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
};

class Device
{
public:
  explicit Device(const DeviceCreateInfo&);
  ~Device() noexcept;

  Device(const Device &) = delete;
  Device& operator=(const Device &) = delete;

  void beginFrame();
  void endFrame();
  void execute(FrameGraph& fg);
  [[nodiscard]] Format swapchainFormat(SwapchainHandle handle) const;

  void waitIdle() const;

  [[nodiscard]] SwapchainHandle createSwapchain(const SwapchainCreateInfo& info);
  [[nodiscard]] SwapchainHandle defaultSwapchain() const noexcept;
  [[nodiscard]] Extent2D swapchainExtent(SwapchainHandle handle) const;
  void destroySwapchain(SwapchainHandle handle) noexcept;

  [[nodiscard]] BufferHandle createBuffer(const BufferDesc& desc);
  void destroyBuffer(BufferHandle handle) noexcept;
  void uploadBufferData(BufferHandle handle, const void* data, uint64_t size);

  [[nodiscard]] void* mapBuffer(BufferHandle handle);
  void unmapBuffer(BufferHandle handle);

  [[nodiscard]] ShaderHandle createShader(const ShaderDesc& desc);
  void destroyShader(ShaderHandle handle) noexcept;

  [[nodiscard]] PipelineHandle createGraphicsPipeline(const GraphicsPipelineDesc& desc);
  void destroyPipeline(PipelineHandle handle) noexcept;

  struct Impl;

private:
  std::unique_ptr<Impl> m_Impl;
};

}
