#pragma once

#include "SkyRHI/Handle.h"
#include "Core/Log.h"

namespace Sky::RHI
{

constexpr uint64_t packHandleId(const uint32_t index, const uint32_t generation) noexcept
{
  return (static_cast<uint64_t>(generation) << 32) | static_cast<uint64_t>(index);
}

constexpr uint32_t unpackIndex(const uint64_t id) noexcept
{
  return static_cast<uint32_t>(id);
}

constexpr uint32_t unpackGeneration(const uint64_t id) noexcept
{
  return static_cast<uint32_t>(id >> 32);
}

template<typename HandleType, typename ResourceType>
class HandleAllocator
{
public:
  HandleAllocator() = default;
  ~HandleAllocator() = default;

  HandleAllocator(const HandleAllocator&) = delete;
  HandleAllocator& operator=(const HandleAllocator&) = delete;

  [[nodiscard]] HandleType allocate(std::unique_ptr<ResourceType> resource)
  {
    std::lock_guard lock(m_Mutex);

    if (!m_FreeIndices.empty())
    {
      const uint32_t index = m_FreeIndices.back();
      m_FreeIndices.pop_back();

      auto& slot = m_Slots[index];
      slot.generation += 1;
      slot.resource = std::move(resource);

      return HandleType {packHandleId(index, slot.generation)};
    }

    const auto index = static_cast<uint32_t>(m_Slots.size());
    m_Slots.push_back({std::move(resource), 1 });
    return HandleType {packHandleId(index, 1)};
  }

  void deallocate(HandleType handle) noexcept
  {
    std::lock_guard lock(m_Mutex);

    const uint32_t index = unpackIndex(handle.id);
    const uint32_t generation = unpackGeneration(handle.id);

    if (index >= m_Slots.size()
      || generation != m_Slots[index].generation
      || m_Slots[index].resource == nullptr)
    {
      SKY_RHI_WARN("Stale or invalid handle passed to deallocate (id={})", handle.id);
      return;
    }

    m_Slots[index].resource.reset();
    m_FreeIndices.push_back(index);
  }

  [[nodiscard]] ResourceType* resolve(HandleType handle) const noexcept
  {
    std::lock_guard lock(m_Mutex);

    const uint32_t index = unpackIndex(handle.id);
    const uint32_t generation = unpackGeneration(handle.id);

    if (index >= m_Slots.size()
      || generation != m_Slots[index].generation
      || m_Slots[index].resource == nullptr)
      return nullptr;

    return m_Slots[index].resource.get();
  }

  [[nodiscard]] bool isValid(HandleType handle) const noexcept
  {
    return resolve(handle) != nullptr;
  }

private:
  struct Slot
  {
    std::unique_ptr<ResourceType> resource;
    uint32_t generation = 0;
  };

  mutable std::mutex m_Mutex;

  std::vector<Slot> m_Slots;
  std::vector<uint32_t> m_FreeIndices;
};

}