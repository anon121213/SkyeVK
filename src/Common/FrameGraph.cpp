#include "skypch.h"
#include "SkyRHI/FrameGraph.h"

#include <unistd.h>

namespace { std::unique_ptr<void, void(*)(void*)> makeEmptyData() { return { nullptr, [](void*){} }; } }

namespace Sky::RHI
{

TextureHandle FGResources::getTexture(FGResource resource) const noexcept
{
  (void)resource;
  return {};
}

FGResource PassBuilder::importSwapchain(SwapchainHandle swapchain)
{
  return m_Graph->registerImportedSwapchain(swapchain);
}

FGResource PassBuilder::createTexture(const char* name, const FGTextureDesc& desc)
{
  return m_Graph->registerTransientTexture(name, desc);
}

FGResource PassBuilder::read(const FGResource resource)
{
  m_Graph->recordRead(m_PassIndex, resource);
  return resource;
}

FGResource PassBuilder::writeColor(const FGResource resource)
{
  m_Graph->recordWrite(m_PassIndex, resource, FGAttachmentType::Color);
  return resource;
}

FGResource PassBuilder::writeDepth(const FGResource resource)
{
  m_Graph->recordWrite(m_PassIndex, resource, FGAttachmentType::Depth);
  return resource;
}

struct FGResourceEntry
{
  std::string name;
  bool imported = false;
  SwapchainHandle importedSwapchain;
  FGTextureDesc desc;
};

struct FGResourceWrite
{
  FGResource resource;
  FGAttachmentType type;
};

struct FrameGraph::Pass
{
  std::string     name;
  ErasedExecuteFn execute;
  ErasedData      data;

  std::vector<FGResource> reads;
  std::vector<FGResourceWrite> writes;
};

struct FrameGraph::Impl
{
  Device*                      device = nullptr;
  std::vector<Pass>            passes;
  std::vector<FGResourceEntry> resources;
  std::vector<uint32_t>        executionOrder;
};

FrameGraph::FrameGraph(Device& device)
  : m_Impl(std::make_unique<Impl>())
{
  m_Impl->device = &device;
}

FrameGraph::~FrameGraph() = default;


void FrameGraph::compile()
{
  constexpr uint32_t kNoProducer = UINT32_MAX;

  const size_t passCount = m_Impl->passes.size();
  const size_t resCount  = m_Impl->resources.size();

  std::vector<uint32_t> producer(resCount, kNoProducer);
  std::vector<uint32_t> resourceRef(resCount, 0);
  std::vector<uint32_t> passRef(passCount, 0);
  std::vector<bool>     culled(passCount, false);
  std::vector<uint32_t> inDegree(passCount, 0);
  std::vector<std::vector<uint32_t>> adj(passCount);

  for (uint32_t i = 0; i < passCount; i++)
    for (const auto& writer : m_Impl->passes[i].writes)
      producer[writer.resource.index] = i;

  for (const auto& pass : m_Impl->passes)
    for (const auto& r : pass.reads)
      resourceRef[r.index]++;

  for (uint32_t r = 0; r < resCount; ++r)
    if (m_Impl->resources[r].imported)
      resourceRef[r]++;

  for (uint32_t p = 0; p < passCount; ++p)
    passRef[p] = static_cast<uint32_t>(m_Impl->passes[p].writes.size());

  std::vector<uint32_t> stack;
  for (uint32_t r = 0; r < resCount; ++r)
    if (resourceRef[r] == 0)
      stack.push_back(r);

  while (!stack.empty())
  {
    auto r = stack.back();
    stack.pop_back();
    auto prod = producer[r];

    if (prod == kNoProducer)
      continue;

    if (--passRef[prod] == 0)
    {
      culled[prod] = true;
      for (const auto& rr : m_Impl->passes[prod].reads)
        if (--resourceRef[rr.index] == 0)
          stack.push_back(rr.index);
    }
  }

  for (uint32_t b = 0; b < passCount; ++b)
  {
    if (culled[b]) continue;

    for (const auto& r : m_Impl->passes[b].reads)
    {
      const uint32_t a = producer[r.index];
      if (a == kNoProducer || culled[a]) continue;

      adj[a].push_back(b);
      inDegree[b]++;
    }
  }

  m_Impl->executionOrder.clear();

  std::vector<uint32_t> ready;
  for (uint32_t p = 0; p < passCount; ++ p)
    if (!culled[p] && inDegree[p] == 0)
      ready.push_back(p);

  while (!ready.empty())
  {
    const uint32_t p = ready.back();
    ready.pop_back();
    m_Impl->executionOrder.push_back(p);

    for (const uint32_t q : adj[p])
      if (--inDegree[q] == 0)
        ready.push_back(q);
  }
}

void FrameGraph::execute(CommandList& cmd)
{
  FGResources resources;
  for (const uint32_t passIndex : m_Impl->executionOrder)
    m_Impl->passes[passIndex].execute(resources, cmd);
}

uint32_t FrameGraph::beginPass(const char* name)
{
  const uint32_t index = static_cast<uint32_t>(m_Impl->passes.size());
  m_Impl->passes.emplace_back(Pass
  {
    .name = name,
    .data = makeEmptyData()
  });
  return index;
}

void FrameGraph::finalizePass(uint32_t passIndex, ErasedExecuteFn execute, ErasedData data)
{
  m_Impl->passes[passIndex].execute = std::move(execute);
  m_Impl->passes[passIndex].data = std::move(data);
}

FGResource FrameGraph::registerImportedSwapchain(SwapchainHandle swapchainHandle)
{
  FGResourceEntry entry{};
  entry.imported = true;
  entry.importedSwapchain = swapchainHandle;
  const uint32_t index = static_cast<uint32_t>(m_Impl->resources.size());
  m_Impl->resources.push_back(entry);
  return FGResource{index};
}

FGResource FrameGraph::registerTransientTexture(const char* name, const FGTextureDesc& desc)
{
  FGResourceEntry entry{};
  entry.imported = false;
  entry.desc = desc;
  const uint32_t index = static_cast<uint32_t>(m_Impl->resources.size());
  m_Impl->resources.push_back(entry);
  return FGResource{index};
}

void FrameGraph::recordRead(uint32_t passIndex, FGResource resource)
{
  m_Impl->passes[passIndex].reads.push_back({resource});
}

void FrameGraph::recordWrite(uint32_t passIndex, FGResource resource, FGAttachmentType type)
{
  m_Impl->passes[passIndex].writes.push_back({resource, type});
}

}