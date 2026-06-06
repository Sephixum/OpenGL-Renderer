#pragma once

#include "Utils/UniqueHandle.hpp"

#include <glad.h>

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

namespace glr
{

  enum struct TextureFormat 
  {
    // 8‑bit unsigned normalized (LDR colour)
    R8unorm      = GL_R8,
    Rg8unorm     = GL_RG8,
    Rgb8unorm    = GL_RGB8,
    Rgba8unorm   = GL_RGBA8,

    // 16‑bit float (high precision for normals, positions)
    Rgba16f       = GL_RGBA16F,
    Rgb16f        = GL_RGB16F,
    R16f          = GL_R16F,

    // 32‑bit float (optional, for ultra‑precise position)
    Rgba32f    = GL_RGBA32F,

    // Depth / Stencil
    Depth24stencil8 = GL_DEPTH24_STENCIL8,
    Depth32f         = GL_DEPTH_COMPONENT32F,
  };

  struct TextureCreateInfo
  {
    std::uint32_t                         width;
    std::uint32_t                         height;
    TextureFormat                         format;
    class Sampler const&                  sampler;
    std::optional<std::vector<std::byte>> data             = std::nullopt;
    bool                                  generate_mipmaps = false;
  };

  class Texture
  {
    UniqueHandle<::GLuint> _id;

    public:
      Texture(TextureCreateInfo const& info, std::string_view name = "Unknown Texture");
      [[nodiscard]] auto GetID() const -> ::GLuint;
  };

}
