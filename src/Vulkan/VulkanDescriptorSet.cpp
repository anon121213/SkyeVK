#include "skypch.h"
#include "VulkanDescriptorSet.h"

VulkanDescriptorSet::VulkanDescriptorSet(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout layout)
  : m_Device(device), m_Pool(pool)
{
  VkDescriptorSetAllocateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  info.descriptorPool = pool;
  info.descriptorSetCount = 1;
  info.pSetLayouts = &layout;

  SKY_RHI_VK_CHECK(vkAllocateDescriptorSets(m_Device, &info, &m_Set),
    "Failed to allocate descriptor set");
}

VulkanDescriptorSet::~VulkanDescriptorSet() noexcept
{
}