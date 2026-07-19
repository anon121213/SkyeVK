#include "VulkanImage.h"

VulkanImage::VulkanImage(VmaAllocator allocator, VkDevice device, VkFormat format, uint32_t width,
                         uint32_t height, VkImageUsageFlags usage, VkImageAspectFlags aspect)
                           : m_Allocator(allocator), m_Device(device)
{
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.format = format;
  imageInfo.extent = { width, height, 1 };
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.usage = usage;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  VmaAllocationCreateInfo allocInfo{};
  allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

  SKY_RHI_VK_CHECK(vmaCreateImage(m_Allocator, &imageInfo, &allocInfo,
                                  &m_Image, &m_Allocation, nullptr),
                                  "Failed to create image");

  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = m_Image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange = { aspect, 0, 1, 0, 1 };

  SKY_RHI_VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_View),
                   "Failed to create image view");
}

VulkanImage::~VulkanImage() noexcept
{
  vkDestroyImageView(m_Device, m_View, nullptr);
  vmaDestroyImage(m_Allocator, m_Image, m_Allocation);
}