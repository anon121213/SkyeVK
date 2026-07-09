#include "skypch.h"

#include "VulkanDevice.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"

VulkanSwapchain::VulkanSwapchain(const VulkanDevice& device, const VulkanSurface& surface,
                                 const uint32_t width, const uint32_t height)
{
  m_Device = device.handle();
  VkSurfaceCapabilitiesKHR caps;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physicalDevice(), surface.handle(), &caps);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice(), surface.handle(), &formatCount, nullptr);
  std::vector<VkSurfaceFormatKHR> formats(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice(), surface.handle(), &formatCount, formats.data());

  uint32_t modesCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device.physicalDevice(), surface.handle(), &modesCount, nullptr);
  std::vector<VkPresentModeKHR> modes(modesCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(device.physicalDevice(), surface.handle(), &modesCount, modes.data());

  const VkSurfaceFormatKHR format = chooseSurfaceFormat(formats);
  const VkPresentModeKHR presentMode = chooseSurfacePresentMode(modes);
  const VkExtent2D extent = chooseSurfaceExtent(caps, width, height);

  uint32_t imageCount = caps.minImageCount + 1;

  if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
    imageCount = caps.maxImageCount;

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface.handle();
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = format.format;
  createInfo.imageColorSpace = format.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.preTransform = caps.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  SKY_RHI_VK_CHECK(vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_Swapchain),
               "Failed to create Vulkan swapchain");

  m_ImageFormat = format.format;
  m_Extent = extent;

  createImageViews();

  SKY_RHI_INFO("Swapchain created: {} images, {}x{}", m_Images.size(), m_Extent.width, m_Extent.height);
}

VulkanSwapchain::~VulkanSwapchain() noexcept
{
  for (const auto view : m_ImageViews)
    vkDestroyImageView(m_Device, view, nullptr);

  vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
}

VkSurfaceFormatKHR VulkanSwapchain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
  for (const auto& format : formats)
    if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      return format;

  return formats[0];
}

VkPresentModeKHR VulkanSwapchain::chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& modes)
{
  for (const auto& mode : modes)
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
      return mode;

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapchain::chooseSurfaceExtent(const VkSurfaceCapabilitiesKHR& caps,
                                                const uint32_t windowWidth,
                                                const uint32_t windowHeight)
{
  if (caps.currentExtent.width != UINT32_MAX)
    return caps.currentExtent;

  VkExtent2D actualExtent = { windowWidth, windowHeight };
  actualExtent.width = std::clamp(actualExtent.width, caps.minImageExtent.width, caps.maxImageExtent.width);
  actualExtent.height = std::clamp(actualExtent.height, caps.minImageExtent.height, caps.maxImageExtent.height);
  return actualExtent;
}

void VulkanSwapchain::createImageViews()
{
  uint32_t actualImageCount;
  vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &actualImageCount, nullptr);
  m_Images.resize(actualImageCount);
  vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &actualImageCount, m_Images.data());

  m_ImageViews.resize(m_Images.size());

  for (size_t i = 0; i < m_Images.size(); ++i)
  {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_Images[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_ImageFormat;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    SKY_RHI_VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageViews[i]),
                 "Failed to create image view");
  }
}
