#include "skypch.h"

#include "SkyRHI/CommandList.h"
#include "SkyRHI/Device.h"
#include "Vulkan/DeviceImpl.h"
#include "VulkanPipeline.h"

namespace Sky::RHI
{

void CommandList::setViewport(const float x, const float y, const float width, const float height) noexcept
{
  VkViewport viewport{};
  viewport.x = x;
  viewport.y = y;
  viewport.width = width;
  viewport.height = height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(static_cast<VkCommandBuffer>(m_NativeCmd), 0, 1, &viewport);
}

void CommandList::setScissor(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height) noexcept
{
  VkRect2D vkRect{};
  vkRect.extent = VkExtent2D{width, height};
  vkRect.offset = VkOffset2D{x, y};
  vkCmdSetScissor(static_cast<VkCommandBuffer>(m_NativeCmd), 0, 1, &vkRect);
}

void CommandList::bindPipeline(PipelineHandle pipeline) noexcept
{
  const auto* impl = static_cast<Device::Impl*>(m_DeviceImpl);
  const VulkanPipeline* vkPipeline = impl->pipelinePool.resolve(pipeline);

  if (!vkPipeline)
  {
    SKY_RHI_WARN("bindPipeline: invalid or stale pipeline handle (id={})", pipeline.id);
    return;
  }

  vkCmdBindPipeline(static_cast<VkCommandBuffer>(m_NativeCmd),
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    vkPipeline->handle());
}

void CommandList::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) noexcept
{
  vkCmdDraw(static_cast<VkCommandBuffer>(m_NativeCmd), vertexCount, instanceCount, firstVertex, firstInstance);
}

}

