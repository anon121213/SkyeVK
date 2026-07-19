#pragma once

#include <cstdint>
#include <cstddef>

namespace Sky::RHI
{

struct ShaderDesc
{
  const uint32_t* code     = nullptr; // Spir-v bytecode
  size_t          codeSize = 0;       // Size in bytes
};

}