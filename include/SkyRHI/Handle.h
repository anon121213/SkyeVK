#pragma once

#include <cstdint>

namespace Sky::RHI
{

constexpr uint64_t kInvalidHandle = 0;

// A strongly-typed opaque handle to a GPU resource.
// The Tag parameter distinguishes handles of different resource kinds at compile time
// without runtime overhead (phantom type pattern).
// Backing storage is a plain uint64_t — POD-safe, cheap to copy/store in ECS components.
template<typename Tag>
struct Handle
{
  uint64_t id = kInvalidHandle;

  [[nodiscard]] constexpr bool valid() const noexcept { return id != kInvalidHandle; }
  constexpr explicit operator bool() const noexcept { return id != kInvalidHandle; }

  constexpr bool operator==(const Handle&) const noexcept = default;
};

struct BufferTag {};              using BufferHandle              = Handle<BufferTag>;
struct TextureTag {};             using TextureHandle             = Handle<TextureTag>;
struct SamplerTag {};             using SamplerHandle             = Handle<SamplerTag>;
struct ShaderTag {};              using ShaderHandle              = Handle<ShaderTag>;
struct PipelineTag {};            using PipelineHandle            = Handle<PipelineTag>;
struct PipelineLayoutTag {};      using PipelineLayoutHandle      = Handle<PipelineLayoutTag>;
struct SwapchainTag {};           using SwapchainHandle           = Handle<SwapchainTag>;
struct CommandListTag {};         using CommandListHandle         = Handle<CommandListTag>;
struct FenceTag {};               using FenceHandle               = Handle<FenceTag>;
struct SemaphoreTag {};           using SemaphoreHandle           = Handle<SemaphoreTag>;
struct DescriptorSetTag {};       using DescriptorSetHandle       = Handle<DescriptorSetTag>;
struct DescriptorSetLayoutTag {}; using DescriptorSetLayoutHandle = Handle<DescriptorSetLayoutTag>;
struct AccelStructureTag {};      using AccelStructureHandle      = Handle<AccelStructureTag>;

}