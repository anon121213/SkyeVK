#pragma once

class VulkanDevice;

class VulkanCommandPool
{
public:
  explicit VulkanCommandPool(const VulkanDevice& device);
  ~VulkanCommandPool() noexcept;

  VulkanCommandPool(const VulkanCommandPool&) = delete;
  VulkanCommandPool& operator=(const VulkanCommandPool&) = delete;

  [[nodiscard]] VkCommandBuffer allocatePrimary() const;
  [[nodiscard]] VkCommandPool handle() const { return m_Pool; }

  void free(VkCommandBuffer cmd) const;

private:
  VkDevice m_Device = VK_NULL_HANDLE;
  VkCommandPool m_Pool = VK_NULL_HANDLE;
};