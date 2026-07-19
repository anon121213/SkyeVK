#include "../../src/skypch.h"

#include "SkyRHI/Device.h"
#include "SkyRHI/FrameGraph.h"
#include "Window.h"

#include <fstream>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
    pipeDesc.vertexStride     = sizeof(Vertex);
    pipeDesc.vertexAttributes = attrs;
    pipeDesc.colorFormat      = device.swapchainFormat(sw);  // matching swapchain format
    pipeDesc.depthFormat      = Sky::RHI::Format::D32_SFLOAT;
    pipeDesc.pushConstantSize = sizeof(glm::mat4);

    const auto pipeline = device.createGraphicsPipeline(pipeDesc);

    const Vertex verts[8] = {
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}},   // 0
      {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},   // 1
      {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},   // 2
      {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},   // 3
      {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},   // 4
      {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}},   // 5
      {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}},   // 6
      {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}},   // 7
    };

    const uint16_t indices[36] = {
      0, 1, 2,  2, 3, 0,   // front  (z = -0.5)
      4, 5, 6,  6, 7, 4,   // back   (z = +0.5)
      0, 3, 7,  7, 4, 0,   // left   (x = -0.5)
      1, 2, 6,  6, 5, 1,   // right  (x = +0.5)
      3, 2, 6,  6, 7, 3,   // top    (y = +0.5)
      0, 1, 5,  5, 4, 0,   // bottom (y = -0.5)
    };

    const auto ib = device.createBuffer({
      sizeof(indices),
      Sky::RHI::BufferUsage::Index | Sky::RHI::BufferUsage::TransferDst,
      Sky::RHI::MemoryType::GpuOnly });

    device.uploadBufferData(ib, indices, sizeof(indices));

    const auto vb = device.createBuffer({
       sizeof(verts),
       Sky::RHI::BufferUsage::Vertex | Sky::RHI::BufferUsage::TransferDst,
       Sky::RHI::MemoryType::GpuOnly });
    device.uploadBufferData(vb, verts, sizeof(verts));

    struct TriData
    {
      Sky::RHI::FGResource backbuffer;
      Sky::RHI::FGResource depth;
    };

    while (!window.shouldClose())
    {
      Window::pollEvents();

      device.beginFrame();

      const auto extent = device.swapchainExtent(sw);

      const float time   = static_cast<float>(glfwGetTime());
      const float aspect = static_cast<float>(extent.width) / static_cast<float>(extent.height);

      glm::mat4 model = glm::rotate(glm::mat4(1.0f), time,
                              glm::normalize(glm::vec3(0.5f, 1.0f, 0.0f)));

      glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f),
                             glm::vec3(0.0f, 0.0f, 0.0f),
                                glm::vec3(0.0f, 1.0f, 0.0f));

      glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f);
      proj[1][1] *= -1.0f;

      glm::mat4 mvp = proj * view * model;

      Sky::RHI::FrameGraph fg(device);
      fg.addRasterPass<TriData>("Triangle",
        [&](Sky::RHI::PassBuilder& b, TriData& d) {
          d.backbuffer = b.writeColor(b.importSwapchain(sw));
          d.depth = b.writeDepth(b.createTexture("deph",
            { Sky::RHI::Format::D32_SFLOAT, extent.width, extent.height}));
        },
        [&](const TriData&, Sky::RHI::FGResources&, Sky::RHI::CommandList& cmd) {
          cmd.bindPipeline(pipeline);
          cmd.pushConstants(&mvp, sizeof(mvp));
          cmd.bindVertexBuffer(vb);
          cmd.bindIndexBuffer(ib, Sky::RHI::IndexType::UInt16);
          cmd.setViewport(0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height));
          cmd.setScissor(0, 0, extent.width, extent.height);
          cmd.drawIndexed(sizeof(indices) / sizeof(uint16_t));
        });

      fg.compile();
      device.execute(fg);
      device.endFrame();
    }

    device.waitIdle();

    device.destroyBuffer(ib);
    device.destroyBuffer(vb);
  }
  catch (const std::exception& e)
  {
    SKY_RHI_CRITICAL("Fatal error: {}", e.what());
    return -1;
  }

  Sky::RHI::Log::shutdown();
  return 0;
}
