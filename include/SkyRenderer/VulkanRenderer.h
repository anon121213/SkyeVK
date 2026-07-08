#pragma once
#include "VulkanCommandPool.h"

class VulkanDevice;
class VulkanSwapchain;
class VulkanRenderPass;
class VulkanFramebuffers;
class VulkanPipeline;
class VulkanCommandPool;

class VulkanRenderer
{
public:
  VulkanRenderer(const VulkanDevice& device,
                 const VulkanSwapchain& swapchain,
                 const VulkanRenderPass& renderPass,
                 const VulkanFramebuffers& framebuffers,
                 const VulkanPipeline& pipeline,
                 const VulkanCommandPool& commandPool);
  ~VulkanRenderer() noexcept;

  VulkanRenderer(const VulkanRenderer&) = delete;
  VulkanRenderer& operator=(const VulkanRenderer&) = delete;

  void drawFrame();
  void waitIdle() const;

private:
  void recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex) const;

  VkDevice m_Device = VK_NULL_HANDLE;
  VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
  VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
  VkRenderPass m_RenderPass = VK_NULL_HANDLE;
  VkPipeline m_Pipeline = VK_NULL_HANDLE;

  VkSemaphore m_ImageAvailable = VK_NULL_HANDLE;
  VkSemaphore m_RenderFinished = VK_NULL_HANDLE;
  VkFence m_InFlight = VK_NULL_HANDLE;

  VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

  const VulkanSwapchain& m_SwapchainRef;
  const VulkanFramebuffers& m_FramebuffersRef;
};