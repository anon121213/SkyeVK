#include "SkyRenderer/VulkanCommandPool.h"

#include <stdexcept>

VulkanCommandPool::VulkanCommandPool(const VulkanDevice& device)
{
  m_Device = device.handle();

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = device.graphicFamilyIndex();

  if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_Pool) != VK_SUCCESS)
    throw std::runtime_error("Failed to create command pool");
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
  if (vkAllocateCommandBuffers(m_Device, &allocInfo, &cmd) != VK_SUCCESS)
    throw std::runtime_error("Failed to allocate command buffer");

  return cmd;
}