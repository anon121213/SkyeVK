#pragma once

#include "SkyRHI/DescriptorSetlayout.h"

#include <vulkan/vulkan.h>

class VulkanDescriptorSetLayout
{
public:
  VulkanDescriptorSetLayout(VkDevice device, const Sky::RHI::DescriptorSetLayoutDesc& desc);
  ~VulkanDescriptorSetLayout() noexcept;

  VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout&) = delete;
  VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout&) = delete;

  [[nodiscard]]  VkDescriptorSetLayout handle() const { return m_Layout; }

private:
  VkDevice              m_Device = VK_NULL_HANDLE;
  VkDescriptorSetLayout m_Layout = VK_NULL_HANDLE;
};