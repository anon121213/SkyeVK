#pragma once
#include <vulkan/vulkan.h>

class VulkanDevice;

class VulkanRenderPass
{
public:
  VulkanRenderPass(const VulkanDevice& device, VkFormat colorFormat);
  ~VulkanRenderPass();

  VulkanRenderPass(const VulkanRenderPass&) = delete;
  VulkanRenderPass& operator=(const VulkanRenderPass&) = delete;

  [[nodiscard]] VkRenderPass handle() const { return m_RenderPass; }

private:
  VkDevice m_Device = VK_NULL_HANDLE;
  VkRenderPass m_RenderPass = VK_NULL_HANDLE;
};