#pragma once

#include "IGLResource.hpp"
#include "Utils/Utils.hpp"

#include <glad.h>
#include <optional>
#include <string_view>

namespace glr
{

  enum struct SamplerMinFilterType
  {
    Linear               = GL_LINEAR,
    Nearest              = GL_NEAREST,
    LinearMipMapLinear   = GL_LINEAR_MIPMAP_LINEAR,
    LinearMipMapNearest  = GL_LINEAR_MIPMAP_NEAREST,
    NearestMipMapNearest = GL_NEAREST_MIPMAP_NEAREST,
    NearestMipMapLinear  = GL_NEAREST_MIPMAP_LINEAR,
  };

  enum struct SamplerMagFilterType
  {
    Linear  = GL_LINEAR,
    Nearest = GL_NEAREST
  };
  
  enum struct SamplerWrapModeType
  {
    Repeat         = GL_REPEAT,
    ClampToEdge    = GL_CLAMP_TO_EDGE,
    MirroredRepeat = GL_MIRRORED_REPEAT
  };

  enum struct SamplerCompareFuncType 
  {
    Never    = GL_NEVER,
    Less     = GL_LESS,
    Equal    = GL_EQUAL,
    Lequal   = GL_LEQUAL,
    Greater  = GL_GREATER,
    NotEqual = GL_NOTEQUAL,
    Gequal   = GL_GEQUAL,
    Always   = GL_ALWAYS,
  };

  struct SamplerCreateInfo
  {
    SamplerMinFilterType                  min_filter;
    SamplerMagFilterType                  mag_filter;
    SamplerWrapModeType                   wrap_s;
    SamplerWrapModeType                   wrap_t;
    std::optional<f32>                    anisotropy_samples = std::nullopt;
    std::optional<SamplerCompareFuncType> compare_func       = std::nullopt;
  };

  class Sampler : public IGLResource
  {
    public:
      Sampler(SamplerCreateInfo const& info, std::string_view name = "Unknown Sampler");
  };

}
