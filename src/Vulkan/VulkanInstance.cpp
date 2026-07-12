#include "skypch.h"

#include "VulkanInstance.h"

namespace
{
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    const VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void*)
{
  const bool isGeneral = (type & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) != 0;

  if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    SKY_RHI_ERROR("[Vulkan] {}", data->pMessage);
  else if ((severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) && !isGeneral)
    SKY_RHI_WARN("[Vulkan] {}", data->pMessage);
  else
    SKY_RHI_TRACE("[Vulkan] {}", data->pMessage);

  return VK_FALSE;
}

bool validationLayerAvailable()
{
  uint32_t count = 0;
  vkEnumerateInstanceLayerProperties(&count, nullptr);
  std::vector<VkLayerProperties> layers(count);
  vkEnumerateInstanceLayerProperties(&count, layers.data());

  for (const auto& layer : layers)
    if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0)
      return true;

  return false;
}

void fillDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& info)
{
  info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                       | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                   | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                   | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  info.pfnUserCallback = debugCallback;
}
}

VulkanInstance::VulkanInstance(const std::string& appName, const std::string& engineName,
                               const std::vector<const char*>& requiredExtensions,
                               const bool enableValidation)
{
  if (volkInitialize() != VK_SUCCESS)
  {
    SKY_RHI_ERROR("Failed to initialize volk (Vulkan loader not found)");
    throw std::runtime_error("Failed to initialize volk");
  }

  const bool useValidation = enableValidation && validationLayerAvailable();
  if (enableValidation && !useValidation)
    SKY_RHI_WARN("Validation requested but VK_LAYER_KHRONOS_validation unavailable — running without it");

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

  std::vector<const char*> layers;
  if (useValidation)
  {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    layers.push_back("VK_LAYER_KHRONOS_validation");
  }

  VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
  fillDebugMessengerInfo(debugInfo);

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();
  createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
  createInfo.ppEnabledLayerNames = layers.data();
  createInfo.pNext = useValidation ? &debugInfo : nullptr;

  SKY_RHI_VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_Instance),
               "Failed to create Vulkan instance");

  volkLoadInstance(m_Instance);

  if (useValidation)
  {
    SKY_RHI_VK_CHECK(vkCreateDebugUtilsMessengerEXT(m_Instance, &debugInfo, nullptr, &m_DebugMessenger),
                 "Failed to create debug messenger");
    SKY_RHI_INFO("Validation layers enabled");
  }

  SKY_RHI_INFO("Vulkan instance created ({} extensions)", extensions.size());
}

VulkanInstance::~VulkanInstance()
{
  if (m_DebugMessenger != VK_NULL_HANDLE)
    vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);

  vkDestroyInstance(m_Instance, nullptr);
}
