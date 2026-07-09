#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

class VulkanInstance
{
public:
  explicit VulkanInstance(const std::string& appName = "SkyRenderer", const std::string& engineName = "SkyGraphicEngine",
                 const std::vector<const char*>& requiredExtensions = {});
  ~VulkanInstance();

  VulkanInstance(const VulkanInstance &) = delete;
  VulkanInstance &operator=(const VulkanInstance &) = delete;

  [[nodiscard]] VkInstance handle() const { return m_Instance; }

private:
  VkInstance m_Instance = VK_NULL_HANDLE;
};

