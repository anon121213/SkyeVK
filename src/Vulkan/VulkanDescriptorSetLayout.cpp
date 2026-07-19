#include "VulkanDescriptorSetLayout.h"

#include "VulkanTranslate.h"
VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VkDevice device, const Sky::RHI::DescriptorSetLayoutDesc& desc)
  : m_Device(device)
{
  std::vector<VkDescriptorSetLayoutBinding> bindings;
  for (const auto& b : desc.bindings)
  {
    VkDescriptorSetLayoutBinding vb{};
    vb.binding = b.binding;
    vb.descriptorType = toVkDescriptorType(b.type);
    vb.descriptorCount = 1;
    vb.stageFlags = toVkShaderStage(b.stage);
    bindings.push_back(vb);
  }

  VkDescriptorSetLayoutCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  info.bindingCount = static_cast<uint32_t>(bindings.size());
  info.pBindings = bindings.data();

  SKY_RHI_VK_CHECK(vkCreateDescriptorSetLayout(m_Device, &info, nullptr, &m_Layout),
    "Failed to create descriptor set layout");
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() noexcept
{
  vkDestroyDescriptorSetLayout(m_Device, m_Layout, nullptr);
}