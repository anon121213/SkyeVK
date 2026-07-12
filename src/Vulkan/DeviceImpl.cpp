#include "skypch.h"

#include "DeviceImpl.h"

#include "SkyRHI/FrameGraph.h"

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
  , vertShader(device, info.vertShaderPath)
  , fragShader(device, info.fragShaderPath)
  , trianglePipelineHandle(pipelinePool.allocate(
    std::make_unique<VulkanPipeline>(device, defaultEntry->swapchain.imageFormat(), vertShader, fragShader)))
  , commandPool(device)
{
  commandBuffer = commandPool.allocatePrimary();

  renderFinished.resize(defaultEntry->swapchain.images().size());

  VkSemaphoreCreateInfo semInfo{};
  semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  SKY_RHI_VK_CHECK(vkCreateSemaphore(device.handle(), &semInfo, nullptr, &imageAvailable),
               "Failed to create imageAvailable semaphore");

  for (auto& sem : renderFinished)
    SKY_RHI_VK_CHECK(vkCreateSemaphore(device.handle(), &semInfo, nullptr, &sem),
                 "Failed to create renderFinished semaphore");

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  SKY_RHI_VK_CHECK(vkCreateFence(device.handle(), &fenceInfo, nullptr, &inFlight),
               "Failed to create inFlight fence");

  SKY_RHI_INFO("Device initialized (backend = Vulkan)");
}

Device::Impl::~Impl()
{
  vkDeviceWaitIdle(device.handle());

  vkDestroyFence(device.handle(), inFlight, nullptr);

  for (const auto sem : renderFinished)
    vkDestroySemaphore(device.handle(), sem, nullptr);

  vkDestroySemaphore(device.handle(), imageAvailable, nullptr);
}

void Device::Impl::beginFrame()
{
  vkWaitForFences(device.handle(), 1, &inFlight, VK_TRUE, UINT64_MAX);
  vkResetFences(device.handle(), 1, &inFlight);

  vkAcquireNextImageKHR(device.handle(), defaultEntry->swapchain.handle(), UINT64_MAX,
                        imageAvailable, VK_NULL_HANDLE, &currentImageIndex);

  vkResetCommandBuffer(commandBuffer, 0);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  SKY_RHI_VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo), "Failed to begin command buffer");
}

void Device::Impl::endFrame()
{
  SKY_RHI_VK_CHECK(vkEndCommandBuffer(commandBuffer), "Failed to end command buffer");

  VkSemaphore waitSemaphores[] = { imageAvailable };
  VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  VkSemaphore signalSemaphores[] = { renderFinished[currentImageIndex] };

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  SKY_RHI_VK_CHECK(vkQueueSubmit(device.graphicQueue(), 1, &submitInfo, inFlight),
               "Failed to submit draw command buffer");

  VkSwapchainKHR swapchain = defaultEntry->swapchain.handle();

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &swapchain;
  presentInfo.pImageIndices = &currentImageIndex;

  vkQueuePresentKHR(device.graphicQueue(), &presentInfo);
}

void Device::Impl::recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex)
{
  const VkImage     image     = defaultEntry->swapchain.images()[imageIndex];
  const VkImageView imageView = defaultEntry->swapchain.imageViews()[imageIndex];
  const VkExtent2D  extent    = defaultEntry->swapchain.extent();

  // Transition the acquired swapchain image: UNDEFINED -> COLOR_ATTACHMENT_OPTIMAL.
  VkImageMemoryBarrier toColor{};
  toColor.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  toColor.srcAccessMask = 0;
  toColor.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  toColor.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  toColor.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  toColor.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  toColor.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  toColor.image = image;
  toColor.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

  vkCmdPipelineBarrier(cmd,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      0, 0, nullptr, 0, nullptr, 1, &toColor);

  VkRenderingAttachmentInfoKHR colorAttachment{};
  colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
  colorAttachment.imageView = imageView;
  colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.clearValue.color = {{ 0.0f, 0.0f, 0.05f, 1.0f }};

  VkRenderingInfoKHR renderingInfo{};
  renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
  renderingInfo.renderArea.offset = { 0, 0 };
  renderingInfo.renderArea.extent = extent;
  renderingInfo.layerCount = 1;
  renderingInfo.colorAttachmentCount = 1;
  renderingInfo.pColorAttachments = &colorAttachment;

  vkCmdBeginRenderingKHR(cmd, &renderingInfo);

  {
    CommandList commandList = createCommandList(cmd);
    commandList.bindPipeline(trianglePipelineHandle);
    commandList.setViewport(0.0f, 0.0f,
                            static_cast<float>(extent.width),
                            static_cast<float>(extent.height));
    commandList.setScissor(0, 0, extent.width, extent.height);
    commandList.draw(3);
  }

  vkCmdEndRenderingKHR(cmd);

  // Transition for presentation: COLOR_ATTACHMENT_OPTIMAL -> PRESENT_SRC_KHR.
  VkImageMemoryBarrier toPresent{};
  toPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  toPresent.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  toPresent.dstAccessMask = 0;
  toPresent.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  toPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  toPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  toPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  toPresent.image = image;
  toPresent.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

  vkCmdPipelineBarrier(cmd,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
      0, 0, nullptr, 0, nullptr, 1, &toPresent);
}

void Device::Impl::waitIdle() const
{
  vkDeviceWaitIdle(device.handle());
}

Device::Device(const DeviceCreateInfo& info): m_Impl(std::make_unique<Impl>(info)) {}
Device::~Device() noexcept = default;

void Device::drawFrame()
{
  m_Impl->beginFrame();

  struct TriData { FGResource backbuffer; };
  FrameGraph fg(*this);
  fg.addRasterPass<TriData>("Triangle",
    [&](PassBuilder& b, TriData& d){
      d.backbuffer = b.writeColor(b.importSwapchain(m_Impl->defaultSwapchainHandle));
    },
    [&](const TriData&, FGResources&, CommandList& cmd){
      const auto extent = m_Impl->defaultEntry->swapchain.extent();
      cmd.bindPipeline(m_Impl->trianglePipelineHandle);
      cmd.setViewport(0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height));
      cmd.setScissor(0, 0, extent.width, extent.height);
      cmd.draw(3);
    });

  fg.compile();

  CommandList cmd = m_Impl->createCommandList(m_Impl->commandBuffer);
  fg.execute(cmd);

  m_Impl->endFrame();
}

void Device::waitIdle() const { m_Impl->waitIdle(); }

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
  return m_Impl->defaultSwapchainHandle;
}

}
