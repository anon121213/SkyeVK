#include "../../src/skypch.h"

#include "SkyRHI/Device.h"
#include "SkyRHI/FrameGraph.h"
#include "Window.h"

#include <fstream>

static std::vector<uint32_t> loadSpirv(const std::string& path)
{
  std::ifstream file(path, std::ios::ate | std::ios::binary);
  if (!file.is_open())
    throw std::runtime_error("Failed to open shader: " + path);

  const size_t sizeBytes = file.tellg();
  std::vector<uint32_t> buffer(sizeBytes / sizeof(uint32_t));
  file.seekg(0);
  file.read(reinterpret_cast<char*>(buffer.data()), sizeBytes);
  return buffer;
}

struct Vertex {
  float pos[3];
  float color[3];
};

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
    };

    Sky::RHI::Device device(info);

    auto vertCode = loadSpirv(std::string(SHADER_DIR) + "/triangle.vert.spv");
    auto fragCode = loadSpirv(std::string(SHADER_DIR) + "/triangle.frag.spv");


    // main.cpp — setup (один раз):
    auto vs = device.createShader({ vertCode.data(), vertCode.size() * sizeof(uint32_t) });
    auto fs = device.createShader({ fragCode.data(), fragCode.size() * sizeof(uint32_t) });

    const auto sw = device.defaultSwapchain();

    const std::vector<Sky::RHI::VertexAttribute> attrs = {
      { 0, Sky::RHI::Format::RGB32_SFLOAT, offsetof(Vertex, pos)   },   // location 0 = pos
      { 1, Sky::RHI::Format::RGB32_SFLOAT, offsetof(Vertex, color) },   // location 1 = color
    };

    Sky::RHI::GraphicsPipelineDesc pipeDesc{};
    pipeDesc.vertexShader     = vs;
    pipeDesc.fragmentShader   = fs;
    pipeDesc.vertexStride     = sizeof(Vertex);              // stride = 24
    pipeDesc.vertexAttributes = attrs;
    pipeDesc.colorFormat      = device.swapchainFormat(sw);  // matching swapchain format

    const auto pipeline = device.createGraphicsPipeline(pipeDesc);

    const float verts[]
    {
      0.0f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   // top    — red
      0.5f,  0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   // right  — green
     -0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   // left   — blue
    };

    const auto vb = device.createBuffer({
       sizeof(verts),                                                    // size = 72
       Sky::RHI::BufferUsage::Vertex | Sky::RHI::BufferUsage::TransferDst,
       Sky::RHI::MemoryType::GpuOnly });
    device.uploadBufferData(vb, verts, sizeof(verts));

    struct TriData {Sky::RHI::FGResource backbuffer; };

    while (!window.shouldClose())
    {
      Window::pollEvents();

      device.beginFrame();

      const auto extent = device.swapchainExtent(sw);

      Sky::RHI::FrameGraph fg(device);
      fg.addRasterPass<TriData>("Triangle",
        [&](Sky::RHI::PassBuilder& b, TriData& d) {
          d.backbuffer = b.writeColor(b.importSwapchain(sw));
        },
        [&](const TriData&, Sky::RHI::FGResources&, Sky::RHI::CommandList& cmd) {
          cmd.bindPipeline(pipeline);
          cmd.bindVertexBuffer(vb);
          cmd.setViewport(0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height));
          cmd.setScissor(0, 0, extent.width, extent.height);
          cmd.draw(3);
        });

      fg.compile();
      device.execute(fg);
      device.endFrame();
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
