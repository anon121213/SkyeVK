#include "skypch.h"

#include "VulkanCommandPool.h"
#include "VulkanDevice.h"

VulkanCommandPool::VulkanCommandPool(const VulkanDevice& device)
{
  m_Device = device.handle();

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = device.graphicFamilyIndex();

  SKY_RHI_VK_CHECK(vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_Pool),
               "Failed to create command pool");

  SKY_RHI_INFO("Command pool created");
}

VulkanCommandPool::~VulkanCommandPool() noexcept
{
  vkDestroyCommandPool(m_Device, m_Pool, nullptr);
}

VkCommandBuffer VulkanCommandPool::allocatePrimary() const
{
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = m_Pool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer cmd;
  SKY_RHI_VK_CHECK(vkAllocateCommandBuffers(m_Device, &allocInfo, &cmd),
               "Failed to allocate command buffer");

  return cmd;
}
void VulkanCommandPool::free(VkCommandBuffer cmd) const
{
  vkFreeCommandBuffers(m_Device, m_Pool, 1, &cmd);
}
