#include "skypch.h"

#include "SkyRHI/Device.h"

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
  VulkanPipeline      pipeline;
  VulkanCommandPool   commandPool;
  VulkanRenderer      renderer;

  explicit Impl(const DeviceCreateInfo& info);
  ~Impl() = default;
};

Device::Impl::Impl(const DeviceCreateInfo& info)
  : instance(info.appName, info.engineName, info.requiredInstanceExtensions)
  , surface(instance, [factory = info.surfaceFactory](const VkInstance vkInst) -> VkSurfaceKHR
{
  return static_cast<VkSurfaceKHR>(factory(vkInst));
})
  , device(instance)
  , swapchain(device, surface, info.initialWindowWidth, info.initialWindowHeight)
  , renderPass(device, swapchain.imageFormat())
  , framebuffers(device, swapchain, renderPass)
  , vertShader(device, info.vertShaderPath)
  , fragShader(device, info.fragShaderPath)
  , pipeline(device, renderPass, vertShader, fragShader)
  , commandPool(device)
  , renderer(device, swapchain, renderPass, framebuffers, pipeline, commandPool)
{
  SKY_RHI_INFO("Device initialized (backend = Vulkan)");
}


Device::Device(const DeviceCreateInfo& info): m_Impl(std::make_unique<Impl>(info)) {}
Device::~Device() noexcept = default;

void Device::drawFrame()      { m_Impl->renderer.drawFrame(); }
void Device::waitIdle() const { m_Impl->renderer.waitIdle(); }

}



