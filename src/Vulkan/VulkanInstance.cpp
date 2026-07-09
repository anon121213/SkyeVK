#include "skypch.h"

#include "VulkanInstance.h"

VulkanInstance::VulkanInstance(const std::string& appName, const std::string& engineName, const std::vector<const char*>& requiredExtensions)
{
  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = appName.c_str();
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = engineName.c_str();
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_2;

  if (requiredExtensions.empty())
  {
    SKY_RHI_ERROR("No window extensions provided");
    throw std::runtime_error("No window extensions provided");
  }

  std::vector extensions(requiredExtensions.begin(), requiredExtensions.end());

  extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
  extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();
  createInfo.enabledLayerCount = 0;

  SKY_RHI_VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_Instance),
               "Failed to create Vulkan instance");

  SKY_RHI_INFO("Vulkan instance created ({} extensions)", extensions.size());
}

VulkanInstance::~VulkanInstance()
{
  vkDestroyInstance(m_Instance, nullptr);
}
