#pragma once

#include "Types.h"

namespace Sky::RHI
{

struct SamplerDesc
{
  Filter            minFilter   = Filter::Linear;
  Filter            magFilter   = Filter::Linear;
  SamplerMipmapMode mipmapMode  = SamplerMipmapMode::Linear;
  AddressMode       addressMode = AddressMode::Repeat;
};

}