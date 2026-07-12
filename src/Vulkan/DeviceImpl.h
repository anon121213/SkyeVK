#pragma once

#include "SkyRHI/Device.h"

#include "Common/HandleAllocator.h"

#include "SkyRHI/CommandList.h"
#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanPipeline.h"
#include "VulkanShaderModule.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"

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

  VulkanShaderModule  vertShader;
  VulkanShaderModule  fragShader;

  HandleAllocator<PipelineHandle, VulkanPipeline> pipelinePool;
  PipelineHandle      trianglePipelineHandle;

  VulkanCommandPool   commandPool;

  // Frame lifecycle (moved from the removed VulkanRenderer).
  VkCommandBuffer          commandBuffer  = VK_NULL_HANDLE;
  VkSemaphore              imageAvailable = VK_NULL_HANDLE;
  std::vector<VkSemaphore> renderFinished;   // one per swapchain image
  VkFence                  inFlight       = VK_NULL_HANDLE;
  uint32_t                 currentImageIndex = 0;   // Backbuffer acquired this frame (for FG realization)

  [[nodiscard]] CommandList createCommandList(VkCommandBuffer cmd) noexcept
  {
    return {cmd, this};
  }

  void beginFrame();   // wait fence, acquire -> currentImageIndex, begin command buffer
  void endFrame();     // end command buffer, submit, present
  void waitIdle() const;

  // Temporary hardcoded triangle recording — replaced by FrameGraph::execute in 1e-final.
  void recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex);

  explicit Impl(const DeviceCreateInfo& info);
  ~Impl();
};

}
