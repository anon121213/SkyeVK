#include "skypch.h"

#include "VulkanBuffer.h"

namespace
{
VkBufferUsageFlags toVkBufferUsage(Sky::RHI::BufferUsage usage)
{
  using U = Sky::RHI::BufferUsage;
  VkBufferUsageFlags flags = 0;
  if (has(usage, U::TransferSrc)) flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  if (has(usage, U::TransferDst)) flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  if (has(usage, U::Vertex))      flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  if (has(usage, U::Index))       flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  if (has(usage, U::Uniform))     flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  if (has(usage, U::Storage))     flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  if (has(usage, U::Indirect))    flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
  return flags;
}

VmaAllocationCreateFlags toVmaFlags(Sky::RHI::MemoryType type)
{
  switch (type)
  {
  case Sky::RHI::MemoryType::GpuOnly:  return 0;
  case Sky::RHI::MemoryType::CpuToGpu: return VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
  case Sky::RHI::MemoryType::GpuToCpu: return VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
  case Sky::RHI::MemoryType::CpuOnly:  return VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
  }
  return 0;
}
}

VulkanBuffer::VulkanBuffer(VmaAllocator allocator, const Sky::RHI::BufferDesc& desc)
  : m_Allocator(allocator), m_Size(desc.size)
{
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = desc.size;
  bufferInfo.usage = toVkBufferUsage(desc.usage);
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo allocInfo{};
  allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  allocInfo.flags = toVmaFlags(desc.memory);

  SKY_RHI_VK_CHECK(vmaCreateBuffer(m_Allocator, &bufferInfo, &allocInfo,
                                  &m_Buffer, &m_Allocation, nullptr),
                                  "Failed to create buffer");
}

VulkanBuffer::~VulkanBuffer() noexcept
{
  vmaDestroyBuffer(m_Allocator, m_Buffer, m_Allocation);
}

void* VulkanBuffer::map()
{
  void* data = nullptr;
  SKY_RHI_VK_CHECK(vmaMapMemory(m_Allocator, m_Allocation, &data), "Failed to map buffer");
  return data;
}

void VulkanBuffer::unmap()
{
  vmaUnmapMemory(m_Allocator, m_Allocation);
}