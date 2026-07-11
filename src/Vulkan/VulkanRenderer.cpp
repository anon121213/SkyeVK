#include "skypch.h"

#include "SkyRHI/CommandList.h"
#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "VulkanFramebuffers.h"
#include "VulkanRenderPass.h"
#include "VulkanRenderer.h"

#include "DeviceImpl.h"
#include "VulkanSwapchain.h"

VulkanRenderer::VulkanRenderer(const VulkanDevice& device,
                               const VulkanSwapchain& swapchain,
                               const VulkanRenderPass& renderPass,
                               const VulkanFramebuffers& framebuffers,
                               const VulkanCommandPool& commandPool,
                               Sky::RHI::Device::Impl& impl)
                                 : m_SwapchainRef(swapchain), m_FramebuffersRef(framebuffers)
{
  m_Device = device.handle();
  m_GraphicsQueue = device.graphicQueue();
  m_Swapchain = swapchain.handle();
  m_RenderPass = renderPass.handle();
  m_CommandBuffer = commandPool.allocatePrimary();
  m_DeviceImpl = &impl;

  VkSemaphoreCreateInfo semInfo{};
  semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  SKY_RHI_VK_CHECK(vkCreateSemaphore(m_Device, &semInfo, nullptr, &m_ImageAvailable),
               "Failed to create imageAvailable semaphore");
  SKY_RHI_VK_CHECK(vkCreateSemaphore(m_Device, &semInfo, nullptr, &m_RenderFinished),
               "Failed to create renderFinished semaphore");

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  SKY_RHI_VK_CHECK(vkCreateFence(m_Device, &fenceInfo, nullptr, &m_InFlight),
               "Failed to create inFlight fence");

  SKY_RHI_INFO("Renderer initialized");
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

  SKY_RHI_VK_CHECK(vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlight),
               "Failed to submit draw command buffer");

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

  SKY_RHI_VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo), "Failed to begin command buffer");

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

  {
    Sky::RHI::CommandList commandList = m_DeviceImpl->createCommandList(cmd);
    commandList.bindPipeline(m_DeviceImpl->trianglePipelineHandle);
    commandList.setViewport(0.0f, 0.0f,
                            static_cast<float>(m_SwapchainRef.extent().width),
                            static_cast<float>(m_SwapchainRef.extent().height));
    commandList.setScissor(0, 0,
                           m_SwapchainRef.extent().width,
                           m_SwapchainRef.extent().height);
    commandList.draw(3);
  }

  vkCmdEndRenderPass(cmd);

  SKY_RHI_VK_CHECK(vkEndCommandBuffer(cmd),
               "Failed to end command buffer");
}
