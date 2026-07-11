#pragma once

#include "CommandList.h"
#include "Handle.h"
#include "Types.h"

#include <cstdint>
#include <memory>
#include <functional>
#include <string>
#include <vector>

namespace Sky::RHI
{

struct FGResource
{
  uint32_t index = 0;
};

struct FGTextureDesc
{
  Format format = Format::Undefined;
  uint32_t width = 0;
  uint32_t height = 0;
};

class FGResources
{
public:
  [[nodiscard]] TextureHandle getTexture(FGResource resource) const noexcept;
};

class PassBuilder
{
public:
  [[nodiscard]] FGResource importSwapchain(SwapchainHandle swapchain);
  [[nodiscard]] FGResource createTexture(const char* name, const FGTextureDesc& desc);

  FGResource read(FGResource resource);
  FGResource writeColor(FGResource resource);
  FGResource writeDepth(FGResource resource);

private:
  friend class FrameGraph;
};

class Device;
class FrameGraph
{
public:
  explicit FrameGraph(Device& device);
  ~FrameGraph();

  FrameGraph(const FrameGraph&) = delete;
  FrameGraph& operator=(const FrameGraph&) = delete;

  template<typename PassData>
  const PassData& addRasterPass(
    const char* name,
    const std::function<void(PassBuilder&, PassData&)>& setup,
    std::function<void(const PassData&, FGResources&, CommandList&)> execute)
  {
    auto data = std::make_unique<PassData>();
    PassData* dataPtr = data.get();

    PassBuilder builder;
    setup(builder, *dataPtr);

    auto erasedExecute = [dataPtr, exec = std::move(execute)]
                         (FGResources& res, CommandList& cmd)
    {
      exec(*dataPtr, res, cmd);
    };

    addPassInternal(name, std::move(erasedExecute),
                   std::unique_ptr<void, void(*)(void*)>(
                            data.release(),
                            [](void* p){delete static_cast<PassData*>(p); }));

    return *dataPtr;
  }

  void compile();
  void execute(CommandList& cmd);

private:
  struct Pass;
  struct Impl;
  std::unique_ptr<Impl> m_Impl;

  using ErasedExecuteFn = std::function<void(FGResources&, CommandList&)>;
  using ErasedData = std::unique_ptr<void, void(*)(void*)>;

  void addPassInternal(const char* name, ErasedExecuteFn execute, ErasedData data);
};

}