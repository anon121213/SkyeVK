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

struct Device::Impl
{
  VulkanInstance      instance;
  VulkanSurface       surface;
  VulkanDevice        device;
  VulkanSwapchain     swapchain;
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