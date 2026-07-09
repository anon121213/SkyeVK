#include "skypch.h"

#include "VulkanInstance.h"
#include "VulkanSurface.h"

VulkanSurface::VulkanSurface(const VulkanInstance& instance, const SurfaceFactory& factory)
{
  m_Instance = instance.handle();
  m_Surface = factory(m_Instance);

  if (m_Surface == VK_NULL_HANDLE)
  {
    SKY_RHI_ERROR("Failed to create Vulkan surface");
    throw std::runtime_error("Failed to create Vulkan surface");
  }

  SKY_RHI_INFO("Vulkan surface created");
}

VulkanSurface::~VulkanSurface()
{
  vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
}
