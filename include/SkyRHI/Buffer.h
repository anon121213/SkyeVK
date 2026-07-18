#pragma once

#include "Types.h"

#include <cstdint>

namespace Sky::RHI
{

struct  BufferDesc
{
  uint64_t size = 0;
  BufferUsage usage = BufferUsage::None;
  MemoryType memory = MemoryType::GpuOnly;
};

}