#include "SkyRenderer/VulkanSurface.h"
#include "SkyRenderer/VulkanInstance.h"

#include <stdexcept>

VulkanSurface::VulkanSurface(const VulkanInstance& instance, const SurfaceFactory& factory)
{
  m_Instance = instance.handle();
  m_Surface = factory(m_Instance);

  if (m_Surface == VK_NULL_HANDLE)
    throw std::runtime_error("Failed to create Vulkan surface");
}

VulkanSurface::~VulkanSurface()
{
  vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
}