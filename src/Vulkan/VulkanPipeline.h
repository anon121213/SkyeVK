#pragma once

#include <vulkan/vulkan.h>

class VulkanShaderModule;
class VulkanDevice;

class VulkanPipeline
{
public:
  VulkanPipeline(const VulkanDevice& device,
    VkFormat colorFormat,
    const VulkanShaderModule& vertexShader,
    const VulkanShaderModule& fragmentShader);
  ~VulkanPipeline() noexcept;

  VulkanPipeline(const VulkanPipeline&) = delete;
  VulkanPipeline& operator=(const VulkanPipeline&) = delete;

  [[nodiscard]] VkPipeline handle() const { return m_Pipeline; }
  [[nodiscard]] VkPipelineLayout layout() const { return m_Layout; }

private:
  VkDevice m_Device = VK_NULL_HANDLE;
  VkPipelineLayout m_Layout = VK_NULL_HANDLE;
  VkPipeline m_Pipeline = VK_NULL_HANDLE;
};