#include "skypch.h"

#include "VulkanDevice.h"
#include "VulkanFramebuffers.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapchain.h"

VulkanFramebuffers::VulkanFramebuffers(const VulkanDevice& device, const VulkanSwapchain& swapchain,
                                       const VulkanRenderPass& render_pass)
{
  m_Device = device.handle();
  const auto imageViews = swapchain.imageViews();
  m_Framebuffers.resize(imageViews.size());

  for (size_t i = 0; i < imageViews.size(); ++i)
  {
    VkImageView attachments[] = { imageViews[i] };

    VkFramebufferCreateInfo frameBufferInfo{};
    frameBufferInfo.sType  = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferInfo.renderPass = render_pass.handle();
    frameBufferInfo.attachmentCount = 1;
    frameBufferInfo.pAttachments = attachments;
    frameBufferInfo.width = swapchain.extent().width;
    frameBufferInfo.height = swapchain.extent().height;
    frameBufferInfo.layers = 1;

    SKY_RHI_VK_CHECK(vkCreateFramebuffer(m_Device, &frameBufferInfo, nullptr, &m_Framebuffers[i]),
                 "Failed to create Vulkan framebuffer");
  }

  SKY_RHI_INFO("Framebuffers created ({})", m_Framebuffers.size());
}

VulkanFramebuffers::~VulkanFramebuffers() noexcept
{
  for (auto& framebuffer : m_Framebuffers)
  {
    vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
  }
}
