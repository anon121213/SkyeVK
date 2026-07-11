#pragma once

#include "SkyRHI/Device.h"

#include "Common/HandleAllocator.h"

#include "SkyRHI/CommandList.h"
#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "VulkanFramebuffers.h"
#include "VulkanInstance.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanRenderer.h"
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
  SwapchainHandle defaultSwapchainHandle;
  VulkanSwapchainEntry* defaultEntry = nullptr;

  VulkanRenderPass    renderPass;
  VulkanFramebuffers  framebuffers;
  VulkanShaderModule  vertShader;
  VulkanShaderModule  fragShader;

  HandleAllocator<PipelineHandle, VulkanPipeline> pipelinePool;
  PipelineHandle      trianglePipelineHandle;

  VulkanCommandPool   commandPool;
  VulkanRenderer      renderer;

  [[nodiscard]] CommandList createCommandList(VkCommandBuffer cmd) noexcept
  {
    return {cmd, this};
  }

  explicit Impl(const DeviceCreateInfo& info);
  ~Impl() = default;
};

}