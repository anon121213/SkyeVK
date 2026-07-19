#pragma once

#include <vulkan/vulkan.h>
#include "SkyRHI/Sampler.h"

class VulkanSampler
{
public:
  VulkanSampler(VkDevice device, const Sky::RHI::SamplerDesc& desc);
  ~VulkanSampler() noexcept;

  VulkanSampler(const VulkanSampler&) = delete;
  VulkanSampler& operator=(const VulkanSampler&) = delete;

  [[nodiscard]] VkSampler handle() const { return m_Sampler; }

private:
  VkDevice  m_Device  = VK_NULL_HANDLE;
  VkSampler m_Sampler = VK_NULL_HANDLE;
};