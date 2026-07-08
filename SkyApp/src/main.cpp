#include "SkyRenderer/VulkanCommandPool.h"
#include "SkyRenderer/VulkanDevice.h"
#include "SkyRenderer/VulkanFramebuffers.h"
#include "SkyRenderer/VulkanInstance.h"
#include "SkyRenderer/VulkanPipeline.h"
#include "SkyRenderer/VulkanRenderPass.h"
#include "SkyRenderer/VulkanRenderer.h"
#include "SkyRenderer/VulkanShaderModule.h"
#include "SkyRenderer/VulkanSurface.h"
#include "SkyRenderer/VulkanSwapchain.h"
#include "Window.h"

int main()
{
  const Window window(1920, 1080, "Vulkan test window");
  const auto extensions = Window::getRequiredInstanceExtensions();

  const VulkanInstance instance("SkyRenderer", "SkyGraphicEngine", extensions);

  VulkanSurface surface(instance, [&window](VkInstance inst){
    return window.createSurface(inst);
  });

  VulkanDevice device(instance);
  VulkanSwapchain swapchain(device, surface, 1920, 1080);
  VulkanRenderPass renderPass(device, swapchain.imageFormat());
  VulkanFramebuffers framebuffers(device, swapchain, renderPass);

  VulkanShaderModule vertShader(device, std::string(SHADER_DIR) + "/triangle.vert.spv");
  VulkanShaderModule fragShader(device, std::string(SHADER_DIR) + "/triangle.frag.spv");

  VulkanPipeline pipeline(device, renderPass, vertShader, fragShader);

  VulkanCommandPool commandPool(device);
  VulkanRenderer renderer(device, swapchain, renderPass, framebuffers, pipeline, commandPool);

  while (!window.shouldClose())
  {
    Window::pollEvents();
    renderer.drawFrame();
  }

  renderer.waitIdle();

  return 0;
}