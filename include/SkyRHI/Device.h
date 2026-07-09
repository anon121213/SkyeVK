#pragma once

#include "Types.h"

#include <functional>
#include <string>
#include <vector>
#include <memory>

namespace Sky::RHI
{

using SurfaceFactoryFn = std::function<void*(void* backendInstance)>;

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

private:
  struct Impl;
  std::unique_ptr<Impl> m_Impl;
};

}
