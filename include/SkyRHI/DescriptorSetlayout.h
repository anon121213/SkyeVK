#pragma once

#include "Types.h"

#include <cstdint>
#include <vector>

namespace Sky::RHI
{

struct DescriptorBinding
{
  uint32_t       binding = 0;
  DescriptorType type    = DescriptorType::CombinedImageSampler;
  ShaderStage    stage   = ShaderStage::Fragment;
};

struct DescriptorSetLayoutDesc
{
  std::vector<DescriptorBinding> bindings;
};

}