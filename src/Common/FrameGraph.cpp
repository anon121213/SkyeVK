#include "skypch.h"
#include "SkyRHI/FrameGraph.h"

namespace Sky::RHI
{

TextureHandle FGResources::getTexture(FGResource resource) const noexcept
{
  (void)resource;
  return {};
}

FGResource PassBuilder::importSwapchain(SwapchainHandle swapchain)
{
  (void)swapchain;
  return {};
}

FGResource PassBuilder::createTexture(const char* name, const FGTextureDesc& desc)
{
  (void)name; (void) desc;
  return {};
}

FGResource PassBuilder::read(FGResource resource)        { return resource; }
FGResource PassBuilder::writeColor(FGResource resource)  { return resource; }
FGResource PassBuilder::writeDepth(FGResource resource) { return resource; }

struct FrameGraph::Pass
{
  std::string     name;
  ErasedExecuteFn execute;
  ErasedData      data;
};

struct FrameGraph::Impl
{
  Device* device = nullptr;
  std::vector<Pass> passes;
};

FrameGraph::FrameGraph(Device& device)
  : m_Impl(std::make_unique<Impl>())
{
  m_Impl->device = &device;
}

FrameGraph::~FrameGraph() = default;

void FrameGraph::addPassInternal(const char* name, ErasedExecuteFn execute, ErasedData data)
{
  m_Impl->passes.push_back(Pass{ name, std::move(execute), std::move(data) });
}

void FrameGraph::compile() {}

void FrameGraph::execute(CommandList& cmd)
{
  FGResources resources;
  for (auto& pass : m_Impl->passes)
    pass.execute(resources, cmd);
}

}