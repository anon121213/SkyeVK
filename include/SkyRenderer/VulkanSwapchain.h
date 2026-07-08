#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class VulkanDevice;
class VulkanSurface;

class VulkanSwapchain
{
public:
  VulkanSwapchain(const VulkanDevice& device, const VulkanSurface& surface, uint32_t width, uint32_t height);
  ~VulkanSwapchain() noexcept;

  VulkanSwapchain(const VulkanSwapchain&) = delete;
  VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

  [[nodiscard]] VkSwapchainKHR handle() const { return m_Swapchain; }
  [[nodiscard]] VkFormat imageFormat() const { return m_ImageFormat; }
  [[nodiscard]] VkExtent2D extent() const { return m_Extent; }
  [[nodiscard]] const std::vector<VkImage>& images() const { return m_Images; }
  [[nodiscard]] const std::vector<VkImageView>& imageViews() const { return m_ImageViews; }

private:
  static VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
  static VkPresentModeKHR chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& modes);
  static VkExtent2D chooseSurfaceExtent(const VkSurfaceCapabilitiesKHR& caps, uint32_t windowWidth, uint32_t windowHeight);
  void createImageViews();

private:
  VkDevice m_Device = VK_NULL_HANDLE;
  VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
  VkFormat m_ImageFormat = VK_FORMAT_UNDEFINED;
  VkExtent2D m_Extent{};
  std::vector<VkImage> m_Images;
  std::vector<VkImageView> m_ImageViews;
};