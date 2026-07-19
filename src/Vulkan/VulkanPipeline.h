#pragma once

#include "SkyRHI/Pipeline.h"
#include <vulkan/vulkan.h>

class VulkanShaderModule;
class VulkanDevice;

class VulkanPipeline
{
public:
  VulkanPipeline(const VulkanDevice& device,
                 const Sky::RHI::GraphicsPipelineDesc& desc,
                 VkShaderModule vertexShader, VkShaderModule fragmentShader);
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