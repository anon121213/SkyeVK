#include "skypch.h"

#include "VulkanDevice.h"
#include "VulkanShaderModule.h"

VulkanShaderModule::VulkanShaderModule(const VulkanDevice& device, const uint32_t* code, size_t codeSize)
{
  m_Device = device.handle();

  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = codeSize;
  createInfo.pCode = code;

  SKY_RHI_VK_CHECK(vkCreateShaderModule(m_Device, &createInfo, nullptr, &m_Module),
             "Failed to create Vulkan shader module");
}

VulkanShaderModule::~VulkanShaderModule() noexcept
{
  vkDestroyShaderModule(m_Device, m_Module, nullptr);
}
