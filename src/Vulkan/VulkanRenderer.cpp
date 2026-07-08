#include "SkyRenderer/VulkanRenderer.h"

#include "SkyRenderer/VulkanFramebuffers.h"
#include "SkyRenderer/VulkanPipeline.h"
#include "SkyRenderer/VulkanRenderPass.h"
#include "SkyRenderer/VulkanSwapchain.h"

#include <stdexcept>

VulkanRenderer::VulkanRenderer(const VulkanDevice& device, const VulkanSwapchain& swapchain,
                               const VulkanRenderPass& renderPass,
                               const VulkanFramebuffers& framebuffers,
                               const VulkanPipeline& pipeline, const VulkanCommandPool& commandPool)
                                 :m_SwapchainRef(swapchain), m_FramebuffersRef(framebuffers)
{
  m_Device = device.handle();
  m_GraphicsQueue = device.graphicQueue();
  m_Swapchain = swapchain.handle();
  m_RenderPass = renderPass.handle();
  m_Pipeline = pipeline.handle();
  m_CommandBuffer = commandPool.allocatePrimary();

  VkSemaphoreCreateInfo semInfo{};
  semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  if (vkCreateSemaphore(m_Device, &semInfo, nullptr, &m_ImageAvailable) != VK_SUCCESS ||
      vkCreateSemaphore(m_Device, &semInfo, nullptr, &m_RenderFinished) != VK_SUCCESS)
    throw std::runtime_error("Failed to create semaphores");

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  if (vkCreateFence(m_Device, &fenceInfo, nullptr, &m_InFlight) != VK_SUCCESS)
    throw std::runtime_error("Failed to create fence");
}

VulkanRenderer::~VulkanRenderer() noexcept
{
  vkDestroyFence(m_Device, m_InFlight, nullptr);
  vkDestroySemaphore(m_Device, m_RenderFinished, nullptr);
  vkDestroySemaphore(m_Device, m_ImageAvailable, nullptr);
}

void VulkanRenderer::drawFrame()
{
  vkWaitForFences(m_Device, 1, &m_InFlight, VK_TRUE, UINT64_MAX);
  vkResetFences(m_Device, 1, &m_InFlight);

  uint32_t imageIndex;
  vkAcquireNextImageKHR(m_Device, m_Swapchain, UINT64_MAX, m_ImageAvailable, VK_NULL_HANDLE, &imageIndex);

  vkResetCommandBuffer(m_CommandBuffer, 0);
  recordCommandBuffer(m_CommandBuffer, imageIndex);

  VkSemaphore waitSemaphores[] = { m_ImageAvailable };
  VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  VkSemaphore signalSemaphores[] = { m_RenderFinished };

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &m_CommandBuffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlight) != VK_SUCCESS)
    throw std::runtime_error("Failed to submit draw command buffer");

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &m_Swapchain;
  presentInfo.pImageIndices = &imageIndex;

  vkQueuePresentKHR(m_GraphicsQueue, &presentInfo);
}

void VulkanRenderer::waitIdle() const
{
  vkDeviceWaitIdle(m_Device);
}

void VulkanRenderer::recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex) const
{
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
    throw std::runtime_error("Failed to create command buffer");

  VkClearValue clearColor{};
  clearColor.color = {{ 0.0f, 0.0f, 0.05f, 1.0f }};

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = m_RenderPass;
  renderPassInfo.framebuffer = m_FramebuffersRef.handles()[imageIndex];
  renderPassInfo.renderArea.offset = { 0, 0 };
  renderPassInfo.renderArea.extent = m_SwapchainRef.extent();
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;

  vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(m_SwapchainRef.extent().width);
  viewport.height = static_cast<float>(m_SwapchainRef.extent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(cmd, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = { 0, 0 };
  scissor.extent = m_SwapchainRef.extent();
  vkCmdSetScissor(cmd, 0, 1, &scissor);

  vkCmdDraw(cmd, 3, 1, 0, 0);

  vkCmdEndRenderPass(cmd);

  if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
    throw std::runtime_error("Failed to end command buffer");
}