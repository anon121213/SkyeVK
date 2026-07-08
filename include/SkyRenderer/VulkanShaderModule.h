#pragma once

#include <vulkan/vulkan.h>
#include <string>

class VulkanDevice;

class VulkanShaderModule
{
public:
  VulkanShaderModule(const VulkanDevice& device, const std::string& spvPath);
  ~VulkanShaderModule() noexcept;

  VulkanShaderModule(const VulkanShaderModule&) = delete;
  VulkanShaderModule& operator=(const VulkanShaderModule&) = delete;

  [[nodiscard]] const VkShaderModule handle() const { return m_Module; }

private:
  VkDevice m_Device = VK_NULL_HANDLE;
  VkShaderModule m_Module = VK_NULL_HANDLE;
};