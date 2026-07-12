#include "skypch.h"

#include "DeviceImpl.h"

namespace
{

std::unique_ptr<Sky::RHI::VulkanSwapchainEntry> makeSwapchainEntry(
  const VulkanInstance& instance, const VulkanDevice& device,
  const Sky::RHI::SurfaceFactoryFn& factory, uint32_t width, uint32_t height)
{
  auto vkFactory = [factory](const VkInstance inst) -> VkSurfaceKHR{
    return static_cast<VkSurfaceKHR>(factory(inst));
  };

  return std::make_unique<Sky::RHI::VulkanSwapchainEntry>(instance, device, vkFactory, width, height);
}

}

namespace Sky::RHI
{

Device::Impl::Impl(const DeviceCreateInfo& info)
  : instance(info.appName, info.engineName, info.requiredInstanceExtensions, info.enableValidation)
  , device(instance)
  , defaultSwapchainHandle(swapchainPool.allocate(
    makeSwapchainEntry(instance, device, info.surfaceFactory,
                               info.initialWindowWidth, info.initialWindowHeight)))
  , defaultEntry(swapchainPool.resolve(defaultSwapchainHandle))
  , renderPass(device, defaultEntry->swapchain.imageFormat())
  , framebuffers(device, defaultEntry->swapchain, renderPass)
  , vertShader(device, info.vertShaderPath)
  , fragShader(device, info.fragShaderPath)
  , trianglePipelineHandle(pipelinePool.allocate(
    std::make_unique<VulkanPipeline>(device, defaultEntry->swapchain.imageFormat(), vertShader, fragShader)))
  , commandPool(device)
  , renderer(device, defaultEntry->swapchain, renderPass, framebuffers, commandPool, *this)
{
  SKY_RHI_INFO("Device initialized (backend = Vulkan)");
}

Device::Device(const DeviceCreateInfo& info): m_Impl(std::make_unique<Impl>(info)) {}
Device::~Device() noexcept = default;

void Device::drawFrame()      { m_Impl->renderer.drawFrame(); }
void Device::waitIdle() const { m_Impl->renderer.waitIdle(); }

SwapchainHandle Device::createSwapchain(const SwapchainCreateInfo& info)
{
  return m_Impl->swapchainPool.allocate(
    makeSwapchainEntry(m_Impl->instance, m_Impl->device,
                          info.surfaceFactory, info.width, info.height));
}

void Device::destroySwapchain(SwapchainHandle handle) noexcept
{
  m_Impl->swapchainPool.deallocate(handle);
}

SwapchainHandle Device::defaultSwapchain() const noexcept
{
  return  m_Impl->defaultSwapchainHandle;
}

}



