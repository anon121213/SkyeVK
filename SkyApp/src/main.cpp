#include "../../src/skypch.h"

#include "SkyRHI/Device.h"
#include "Window.h"

int main()
{
  Sky::RHI::Log::init();
  SKY_RHI_INFO("SkyEngine starting up...");

  try
  {
    const Window window(1920, 1080, "Vulkan test window");
    const auto extensions = Window::getRequiredInstanceExtensions();

    const Sky::RHI::DeviceCreateInfo info{
      .backend = Sky::RHI::BackendType::Vulkan,
      .requiredInstanceExtensions = extensions,
      .surfaceFactory = [&window](void* instance) -> void* {
        return window.createSurface(static_cast<VkInstance>(instance));
      },
      .initialWindowWidth = window.width(),
      .initialWindowHeight = window.height(),
      .enableValidation = true,

      .vertShaderPath = std::string(SHADER_DIR) + "/triangle.vert.spv",
      .fragShaderPath = std::string(SHADER_DIR) + "/triangle.frag.spv"
    };

    Sky::RHI::Device device(info);

    while (!window.shouldClose())
    {
      Window::pollEvents();
      device.drawFrame();
    }

    device.waitIdle();
  }
  catch (const std::exception& e)
  {
    SKY_RHI_CRITICAL("Fatal error: {}", e.what());
    return -1;
  }

  Sky::RHI::Log::shutdown();
  return 0;
}
