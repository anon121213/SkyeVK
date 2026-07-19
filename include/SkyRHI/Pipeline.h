#pragma once

#include "Types.h"
#include "Handle.h"

#include <cstdint>
#include <vector>

namespace Sky::RHI
{

struct VertexAttribute
{
  uint32_t location = 0;                 // matches layout(location=N) in shader
  Format   format   = Format::Undefined; // e.g. RGB32_SFLOAT for vec3
  uint32_t offset   = 0;                 // byte offset within the vertex
};

struct GraphicsPipelineDesc
{
  ShaderHandle vertexShader;
  ShaderHandle fragmentShader;

  uint32_t                     vertexStride = 0;                  // bytes per vertex (0 = no vertex input)
  std::vector<VertexAttribute> vertexAttributes;
  Format                       colorFormat = Format::Undefined;  // color attachment format (dynamic rendering)
  // Defaults (will be reworking): topology=TriangleList, cull=None, blend=off, depth=off.
  // Добавим когда понадобится: depthFormat (Шаг 5), push constants (Шаг 6), blend, topology.
};

}