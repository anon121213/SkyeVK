#pragma once

#include <vulkan/vulkan.h>
#include <functional>

class VulkanInstance;
using SurfaceFactory = std::function<VkSurfaceKHR(VkInstance)>;

class VulkanSurface
{
public:
  explicit VulkanSurface(const VulkanInstance& instance, const SurfaceFactory& factory);
  ~VulkanSurface();

  VulkanSurface(const VulkanSurface&) = delete;
  VulkanSurface& operator=(const VulkanSurface&) = delete;

  [[nodiscard]] VkSurfaceKHR handle() const { return m_Surface; }

private:
  VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
  VkInstance m_Instance = VK_NULL_HANDLE;
};