#include "skypch.h"

#include "DeviceImpl.h"

namespace Sky::RHI
{

Device::Impl::Impl(const DeviceCreateInfo& info)
  : instance(info.appName, info.engineName, info.requiredInstanceExtensions)
  , surface(instance, [factory = info.surfaceFactory](const VkInstance vkInst) -> VkSurfaceKHR
              { return static_cast<VkSurfaceKHR>(factory(vkInst)); })
  , device(instance)
  , swapchain(device, surface, info.initialWindowWidth, info.initialWindowHeight)
  , renderPass(device, swapchain.imageFormat())
  , framebuffers(device, swapchain, renderPass)
  , vertShader(device, info.vertShaderPath)
  , fragShader(device, info.fragShaderPath)
  , trianglePipelineHandle(pipelinePool.allocate(
    std::make_unique<VulkanPipeline>(device, renderPass, vertShader, fragShader)))
  , commandPool(device)
  , renderer(device, swapchain, renderPass, framebuffers, commandPool, *this)
{
  SKY_RHI_INFO("Device initialized (backend = Vulkan)");
}

Device::Device(const DeviceCreateInfo& info): m_Impl(std::make_unique<Impl>(info)) {}
Device::~Device() noexcept = default;

void Device::drawFrame()      { m_Impl->renderer.drawFrame(); }
void Device::waitIdle() const { m_Impl->renderer.waitIdle(); }

}



