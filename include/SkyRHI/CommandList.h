#pragma once

#include "Device.h"
#include "Handle.h"
#include "Types.h"

#include <cstdint>

namespace Sky::RHI
{

class FrameGraph;

class CommandList
{
public:
  CommandList() = delete;
  ~CommandList() = default;

  CommandList(const CommandList&) = delete;
  CommandList& operator=(const CommandList&) = delete;

  CommandList(CommandList&&) noexcept = default;
  CommandList& operator=(CommandList&&) noexcept = default;

  void setViewport(float x, float y, float width, float height) noexcept;
  void setScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) noexcept;
  void bindPipeline(PipelineHandle pipeline) noexcept;
  void draw(uint32_t vertexCount,
            uint32_t instanceCount = 1,
            uint32_t firstVertex = 0,
            uint32_t firstInstance = 0) noexcept;
  void bindVertexBuffer(BufferHandle buffer) noexcept;
  void bindIndexBuffer(BufferHandle buffer, IndexType type) noexcept;
  void drawIndexed(uint32_t indexCount,
    uint32_t instanceCount = 1,
    uint32_t firstIndex    = 0,
    int32_t  vertexOffset  = 0,
    uint32_t firstInstance = 0) noexcept;
  void pushConstants(const void* data, uint32_t size) noexcept;

private:
  friend class Device;
  friend class Device::Impl;
  friend class FrameGraph;

  CommandList(void* nativeCmd, void* deviceImplPtr) noexcept
    : m_NativeCmd(nativeCmd), m_DeviceImpl(deviceImplPtr){}

  void* m_NativeCmd           = nullptr;
  void* m_DeviceImpl          = nullptr;
  void* m_BoundPipelineLayout = nullptr;
};

}
