#include "skypch.h"

#include "SkyRHI/CommandList.h"

#include "SkyRHI/Device.h"
#include "Vulkan/DeviceImpl.h"
#include "VulkanDescriptorSet.h"
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

  m_BoundPipelineLayout = vkPipeline->layout();

  vkCmdBindPipeline(static_cast<VkCommandBuffer>(m_NativeCmd),
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    vkPipeline->handle());
}

void CommandList::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) noexcept
{
  vkCmdDraw(static_cast<VkCommandBuffer>(m_NativeCmd), vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandList::bindVertexBuffer(BufferHandle buffer) noexcept
{
  auto* impl = static_cast<Device::Impl*>(m_DeviceImpl);
  VulkanBuffer* buf = impl->bufferPool.resolve(buffer);
  if (!buf)
  {
    SKY_RHI_WARN("bindVertexBuffer: invalid buffer handle (id={})", buffer.id);
    return;
  }

  VkBuffer vkBuf = buf->handle();
  VkDeviceSize offest = 0;
  vkCmdBindVertexBuffers(static_cast<VkCommandBuffer>(m_NativeCmd), 0, 1, &vkBuf, &offest);
}

void CommandList::bindIndexBuffer(BufferHandle buffer, IndexType type) noexcept
{
  auto* impl = static_cast<Device::Impl*>(m_DeviceImpl);
  VulkanBuffer* buf = impl->bufferPool.resolve(buffer);
  if (!buf)
  {
    SKY_RHI_WARN("bindingIndexBuffer: invalid buffer handle (id={})", buffer.id);
    return;
  }

  const VkIndexType vkType = (type == IndexType::UInt16) ? VK_INDEX_TYPE_UINT16
                                                         : VK_INDEX_TYPE_UINT32;

  vkCmdBindIndexBuffer(static_cast<VkCommandBuffer>(m_NativeCmd), buf->handle(), 0, vkType);
}

void CommandList::drawIndexed(const uint32_t indexCount, const uint32_t instanceCount,
                              const uint32_t firstIndex, const int32_t vertexOffset,
                              const uint32_t firstInstance) noexcept
{
  vkCmdDrawIndexed(static_cast<VkCommandBuffer>(m_NativeCmd),
    indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandList::pushConstants(const void* data, uint32_t size) noexcept
{
  vkCmdPushConstants(static_cast<VkCommandBuffer>(m_NativeCmd),
                    static_cast<VkPipelineLayout>(m_BoundPipelineLayout),
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0, size, data);
}

void CommandList::bindDescriptorSet(DescriptorSetHandle setHandle) noexcept
{
  auto* impl = static_cast<Device::Impl*>(m_DeviceImpl);
  VulkanDescriptorSet* set = impl->descriptorSetPool.resolve(setHandle);

  if (!set)
  {
    SKY_RHI_WARN("bindDescriptorSet: invalid handle");
    return;
  }

  VkDescriptorSet vkSet = set->handle();
  vkCmdBindDescriptorSets(static_cast<VkCommandBuffer>(m_NativeCmd),
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    static_cast<VkPipelineLayout>(m_BoundPipelineLayout),
    0, 1, &vkSet, 0, nullptr);
}

}

