#pragma once

#include <vulkan/vulkan.h>

class VulkanDescriptorSet
{
public:
  VulkanDescriptorSet(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout layout);
  ~VulkanDescriptorSet() noexcept;

  VulkanDescriptorSet(const VulkanDescriptorSet&) = delete;
  VulkanDescriptorSet& operator=(const VulkanDescriptorSet&) = delete;

  [[nodiscard]] VkDescriptorSet handle() const { return m_Set; }

private:
  VkDevice         m_Device = VK_NULL_HANDLE;
  VkDescriptorPool m_Pool   = VK_NULL_HANDLE;
  VkDescriptorSet  m_Set    = VK_NULL_HANDLE;
};