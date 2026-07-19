#pragma once

#include "Common/HandleAllocator.h"
#include "SkyRHI/Device.h"
#include "SkyRHI/CommandList.h"
#include "VulkanBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanPipeline.h"
#include "VulkanShaderModule.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"
#include "VulkanImage.h"

namespace Sky::RHI
{

struct VulkanSwapchainEntry
{
  VulkanSurface surface;
  VulkanSwapchain swapchain;

  VulkanSwapchainEntry(const VulkanInstance& instance,
    const VulkanDevice& device,
    const SurfaceFactory& factory,
    const uint32_t width,
    const uint32_t height)
      : surface(instance, factory)
      , swapchain(device, surface, width, height) {}
};

struct Device::Impl
{
  VulkanInstance      instance;
  VulkanDevice        device;

  HandleAllocator<SwapchainHandle, VulkanSwapchainEntry> swapchainPool;
  SwapchainHandle       defaultSwapchainHandle;
  VulkanSwapchainEntry* defaultEntry = nullptr;

  HandleAllocator<PipelineHandle, VulkanPipeline> pipelinePool;

  VulkanCommandPool   commandPool;

  VkCommandBuffer          commandBuffer  = VK_NULL_HANDLE;
  VkSemaphore              imageAvailable = VK_NULL_HANDLE;
  std::vector<VkSemaphore> renderFinished;   // one per swapchain image
  VkFence                  inFlight       = VK_NULL_HANDLE;
  uint32_t                 currentImageIndex = 0;   // Backbuffer acquired this frame (for FG realization)

  HandleAllocator<BufferHandle, VulkanBuffer> bufferPool;

  HandleAllocator<ShaderHandle, VulkanShaderModule> shaderPool;

  std::vector<std::unique_ptr<VulkanImage>> frameTransients;

  [[nodiscard]] CommandList createCommandList(VkCommandBuffer cmd) noexcept
  {
    return {cmd, this};
  }

  void beginFrame();   // wait fence, acquire -> currentImageIndex, begin command buffer
  void endFrame();     // end command buffer, submit, present
  void waitIdle() const;

  void immediateSubmit(const std::function<void(VkCommandBuffer)>& record);

  explicit Impl(const DeviceCreateInfo& info);
  ~Impl();
};

}
