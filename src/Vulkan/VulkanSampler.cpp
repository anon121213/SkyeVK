#include "VulkanSampler.h"

#include "VulkanTranslate.h"

VulkanSampler::VulkanSampler(VkDevice device, const Sky::RHI::SamplerDesc& desc)
  : m_Device(device)
{
  VkSamplerCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  info.magFilter = toVkFilter(desc.magFilter);
  info.minFilter = toVkFilter(desc.minFilter);
  info.mipmapMode = toVkMipmapMode(desc.mipmapMode);
  info.addressModeU = toVkAddressMode(desc.addressMode);
  info.addressModeV = toVkAddressMode(desc.addressMode);
  info.addressModeW = toVkAddressMode(desc.addressMode);
  info.anisotropyEnable = VK_FALSE;
  info.maxLod = VK_LOD_CLAMP_NONE;
  info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

  SKY_RHI_VK_CHECK(vkCreateSampler(m_Device, &info, nullptr, &m_Sampler),
    "Failed to create sampler");
}

VulkanSampler::~VulkanSampler() noexcept
{
  vkDestroySampler(m_Device, m_Sampler, nullptr);
}