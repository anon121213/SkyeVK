#pragma once

#include "VulkanDevice.h"

#include <vector>

class VulkanRenderPass;
class VulkanSwapchain;
class VulkanDevice;

class VulkanFramebuffers
{
public:
  VulkanFramebuffers(const VulkanDevice& device, const VulkanSwapchain& swapchain, const VulkanRenderPass& render_pass);
  ~VulkanFramebuffers() noexcept;

  VulkanFramebuffers(const VulkanFramebuffers&) = delete;
  VulkanFramebuffers& operator=(const VulkanRenderPass&) = delete;

  [[nodiscard]] const std::vector<VkFramebuffer>& handles() const { return m_Framebuffers; }
  [[nodiscard]] size_t count() const { return m_Framebuffers.size(); }

private:
  VkDevice m_Device;
  std::vector<VkFramebuffer> m_Framebuffers;
};