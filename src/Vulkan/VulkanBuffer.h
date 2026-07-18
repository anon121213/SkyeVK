#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "SkyRHI/Buffer.h"

class VulkanBuffer
{
public:
  VulkanBuffer(VmaAllocator allocator, const Sky::RHI::BufferDesc& desc);
  ~VulkanBuffer() noexcept;

  VulkanBuffer(const VulkanBuffer&) = delete;
  VulkanBuffer& operator=(const VulkanBuffer&) = delete;

  [[nodiscard]] VkBuffer handle() const { return m_Buffer; }
  [[nodiscard]] uint64_t size() const { return m_Size; }
  [[nodiscard]] VmaAllocation allocation() const { return m_Allocation; }

  [[nodiscard]] void* map();
  void unmap();

private:
  VmaAllocator  m_Allocator   = VK_NULL_HANDLE;
  VkBuffer      m_Buffer      = VK_NULL_HANDLE;
  VmaAllocation m_Allocation  = VK_NULL_HANDLE;
  uint64_t      m_Size        = 0;
};