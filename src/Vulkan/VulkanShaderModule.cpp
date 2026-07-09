#include "skypch.h"

#include "VulkanDevice.h"
#include "VulkanShaderModule.h"

VulkanShaderModule::VulkanShaderModule(const VulkanDevice& device, const std::string& spvPath)
{
  std::ifstream file(spvPath, std::ios::ate | std::ios::binary);

  if (!file.is_open())
  {
    SKY_RHI_ERROR("Failed to open shader file: {}", spvPath);
    throw std::runtime_error("Failed to open shader file: " + spvPath);
  }

  const size_t fileSize = file.tellg();
  std::vector<char> buffer(fileSize);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  m_Device = device.handle();

  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = buffer.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

  SKY_RHI_VK_CHECK(vkCreateShaderModule(m_Device, &createInfo, nullptr, &m_Module),
               "Failed to create Vulkan shader module");

  SKY_RHI_INFO("Shader loaded: {}", spvPath);
}

VulkanShaderModule::~VulkanShaderModule() noexcept
{
  vkDestroyShaderModule(m_Device, m_Module, nullptr);
}
