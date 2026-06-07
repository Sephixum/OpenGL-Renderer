#pragma once

#include "IGLResource.hpp"
#include <glad.h>
#include <optional>
#include <string_view>

namespace glr
{

  enum struct MinFilterType
  {
    Linear               = GL_LINEAR,
    Nearest              = GL_NEAREST,
    LinearMipMapLinear   = GL_LINEAR_MIPMAP_LINEAR,
    LinearMipMapNearest  = GL_LINEAR_MIPMAP_NEAREST,
    NearestMipMapNearest = GL_NEAREST_MIPMAP_NEAREST,
    NearestMipMapLinear  = GL_NEAREST_MIPMAP_LINEAR,
  };

  enum struct MagFilterType
  {
    Linear  = GL_LINEAR,
    Nearest = GL_NEAREST
  };
  
  enum struct WrapMode
  {
    Repeat         = GL_REPEAT,
    ClampToEdge    = GL_CLAMP_TO_EDGE,
    MirroredRepeat = GL_MIRRORED_REPEAT
  };

  struct SamplerCreateInfo
  {
    MinFilterType        min_filter;
    MagFilterType        mag_filter;
    WrapMode             wrap_s;
    WrapMode             wrap_t;
    std::optional<float> anisotropy_samples = std::nullopt;
  };

  class Sampler : public IGLResource
  {
    public:
      Sampler(SamplerCreateInfo const& info, std::string_view name = "Unknown Sampler");

  };

}
