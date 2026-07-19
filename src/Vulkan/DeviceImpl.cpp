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

std::vector<uint32_t> loadSpirv(const std::string& path)
{
  std::ifstream file(path, std::ios::ate | std::ios::binary);
  if (!file.is_open()) throw std::runtime_error("Failed to open shader: "+ path);
  const size_t sizeBytes = file.tellg();
  std::vector<uint32_t> buf(sizeBytes / sizeof(uint32_t));
  file.seekg(0);
  file.read(reinterpret_cast<char*>(buf.data()), sizeBytes);
  return buf;
}

Sky::RHI::Format toSkyFormat(VkFormat f)
{
  switch (f)
  {
  case VK_FORMAT_B8G8R8A8_SRGB:  return Sky::RHI::Format::BGRA8_SRGB;
  case VK_FORMAT_R8G8B8A8_SRGB:  return Sky::RHI::Format::RGBA8_SRGB;
  case VK_FORMAT_B8G8R8A8_UNORM: return Sky::RHI::Format::BGRA8_UNORM;
  case VK_FORMAT_R8G8B8A8_UNORM: return Sky::RHI::Format::RGBA8_UNORM;
  default:                       return Sky::RHI::Format::Undefined;
  }
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


void Device::Impl::immediateSubmit(const std::function<void(VkCommandBuffer)>& record)
{
  VkCommandBuffer cmd = commandPool.allocatePrimary();

  VkCommandBufferBeginInfo begin{};
  begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(cmd, &begin);

  record(cmd);

  vkEndCommandBuffer(cmd);

  VkSubmitInfo submit{};
  submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit.commandBufferCount = 1;
  submit.pCommandBuffers = &cmd;

  SKY_RHI_VK_CHECK(vkQueueSubmit(device.graphicQueue(), 1, &submit, VK_NULL_HANDLE),
    "Immediate submit failed");

  vkQueueWaitIdle(device.graphicQueue());

  commandPool.free(cmd);
}

void Device::Impl::waitIdle() const
{
  vkDeviceWaitIdle(device.handle());
}

Device::Device(const DeviceCreateInfo& info) : m_Impl(std::make_unique<Impl>(info)) {}

Device::~Device() noexcept = default;

void Device::beginFrame()
{
  m_Impl->beginFrame();
}

void Device::endFrame()
{
  m_Impl->endFrame();
}

void Device::execute(FrameGraph& fg)
{
  CommandList cmd = m_Impl->createCommandList(m_Impl->commandBuffer);
  fg.execute(cmd);
}

Format Device::swapchainFormat(SwapchainHandle handle) const
{
  VulkanSwapchainEntry* entry = m_Impl->swapchainPool.resolve(handle);
  return entry ? toSkyFormat(entry->swapchain.imageFormat()) : Format::Undefined;
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

Extent2D Device::swapchainExtent(SwapchainHandle handle) const
{
  VulkanSwapchainEntry* entry = m_Impl->swapchainPool.resolve(handle);
  if (!entry) return {};
  const auto [width, height] = entry->swapchain.extent();
  return { width, height };
}

BufferHandle Device::createBuffer(const BufferDesc& desc)
{
  return m_Impl->bufferPool.allocate(std::make_unique<VulkanBuffer>(m_Impl->device.allocator(), desc));
}

void Device::destroyBuffer(BufferHandle handle) noexcept
{
  m_Impl->bufferPool.deallocate(handle);
}

void* Device::mapBuffer(BufferHandle handle)
{
  VulkanBuffer* buf = m_Impl->bufferPool.resolve(handle);
  return buf ? buf->map() : nullptr;
}

void Device::unmapBuffer(BufferHandle handle)
{
  if (VulkanBuffer* buf = m_Impl->bufferPool.resolve(handle))
    buf->unmap();
}

void Device::uploadBufferData(BufferHandle handle, const void* data, uint64_t size)
{
  VulkanBuffer* dst = m_Impl->bufferPool.resolve(handle);
  if (!dst)
  {
    SKY_RHI_WARN("uploadBufferData: invalid buffer handle");
    return;
  }

  VulkanBuffer staging(m_Impl->device.allocator(),
    {size, BufferUsage::TransferSrc, MemoryType::CpuOnly});

  SKY_RHI_VK_CHECK(vmaCopyMemoryToAllocation(m_Impl->device.allocator(), data,
    staging.allocation(), 0, size),
    "Failed to copy data to staging buffer");

  m_Impl->immediateSubmit([&](VkCommandBuffer cmd)
  {
    VkBufferCopy region{};
    region.size = size;
    vkCmdCopyBuffer(cmd, staging.handle(), dst->handle(), 1, &region);
  });
}

ShaderHandle Device::createShader(const ShaderDesc& desc)
{
  return m_Impl->shaderPool.allocate(
    std::make_unique<VulkanShaderModule>(m_Impl->device, desc.code, desc.codeSize));
}

void Device::destroyShader(ShaderHandle handle) noexcept
{
  m_Impl->shaderPool.deallocate(handle);
}

PipelineHandle Device::createGraphicsPipeline(const GraphicsPipelineDesc& desc)
{
  VulkanShaderModule* vs = m_Impl->shaderPool.resolve(desc.vertexShader);
  VulkanShaderModule* fs = m_Impl->shaderPool.resolve(desc.fragmentShader);

  if (!vs || !fs)
  {
    SKY_RHI_ERROR("createGraphicsPipeline: invalid shader handle");
    return {};
  }

  return m_Impl->pipelinePool.allocate(
    std::make_unique<VulkanPipeline>(m_Impl->device, desc, vs->handle(), fs->handle()));
}

void Device::destroyPipeline(PipelineHandle handle) noexcept
{
  m_Impl->pipelinePool.deallocate(handle);
}

} // namespace Sky::RHI
