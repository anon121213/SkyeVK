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
  m_RenderFinished.resize(swapchain.images().size());



  VkSemaphoreCreateInfo semInfo{};
  semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  SKY_RHI_VK_CHECK(vkCreateSemaphore(m_Device, &semInfo, nullptr, &m_ImageAvailable),
               "Failed to create imageAvailable semaphore");

  for (auto& sem : m_RenderFinished)
    SKY_RHI_VK_CHECK(vkCreateSemaphore(m_Device, &semInfo, nullptr, &sem),
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

  for (const auto sem : m_RenderFinished)
    vkDestroySemaphore(m_Device, sem, nullptr);

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
  VkSemaphore signalSemaphores[] = { m_RenderFinished[imageIndex] };

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

  const VkImage image = m_SwapchainRef.images()[imageIndex];
  const VkImageView imageView = m_SwapchainRef.imageViews()[imageIndex];
  const VkExtent2D extent = m_SwapchainRef.extent();

  VkImageMemoryBarrier toColor{};
  toColor.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  toColor.srcAccessMask = 0;
  toColor.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  toColor.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  toColor.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  toColor.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  toColor.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  toColor.image = image;
  toColor.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

  vkCmdPipelineBarrier(cmd,
  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    0, 0, nullptr, 0, nullptr, 1, &toColor);

  VkRenderingAttachmentInfoKHR colorAttachment{};
  colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
  colorAttachment.imageView = imageView;
  colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.clearValue.color = {{ 0.0f, 0.0f, 0.05f, 1.0f }};

  VkRenderingInfoKHR renderingInfo{};
  renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
  renderingInfo.renderArea.offset = { 0, 0 };
  renderingInfo.renderArea.extent = extent;
  renderingInfo.layerCount = 1;
  renderingInfo.colorAttachmentCount = 1;
  renderingInfo.pColorAttachments = &colorAttachment;

  vkCmdBeginRenderingKHR(cmd, &renderingInfo);

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

  vkCmdEndRenderingKHR(cmd);

  VkImageMemoryBarrier toPresent{};
  toPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  toPresent.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  toPresent.dstAccessMask = 0;
  toPresent.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  toPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  toPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  toPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  toPresent.image = image;
  toPresent.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

  vkCmdPipelineBarrier(cmd,
  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    0, 0, nullptr, 0, nullptr, 1, &toPresent);

  SKY_RHI_VK_CHECK(vkEndCommandBuffer(cmd),
               "Failed to end command buffer");
}
