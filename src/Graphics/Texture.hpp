#pragma once

#include "IGLResource.hpp"
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
    Depth32f        = GL_DEPTH_COMPONENT32F,
  };

  enum struct TextureType
  {
    Texture2D                   = GL_TEXTURE_2D

    //////// Future maybe ? ;)
    // Texture1D                   = GL_TEXTURE_1D,
    // Texture3D                   = GL_TEXTURE_3D,
    // Texture1DArray              = GL_TEXTURE_1D_ARRAY,
    // Texture2DArray              = GL_TEXTURE_2D_ARRAY,
    // CubeMap                     = GL_TEXTURE_CUBE_MAP,
    // CubeMapArray                = GL_TEXTURE_CUBE_MAP_ARRAY,
    // Texture2DMultisample        = GL_TEXTURE_2D_MULTISAMPLE,
    // Texture2DMultisampleArray   = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
  };



  struct Texture2DCreateInfo
  {
    std::uint32_t                         width;
    std::uint32_t                         height;
    TextureFormat                         format;
    class Sampler const&                  sampler;
    std::optional<std::vector<std::byte>> data          = std::nullopt;
    std::uint32_t                         mipmap_levels = 0;
  };

  template <TextureType T>
  class Texture;

  template <>
  class Texture<TextureType::Texture2D> : public IGLResource
  {
    UniqueHandle<::GLuint64> _bindless_handle;
    std::uint32_t            _width;
    std::uint32_t            _height;
    std::string              _name;

    public:
      Texture(Texture2DCreateInfo const& info, std::string_view name = "Unknown Texture");
      Texture(Texture&&) noexcept = default;
      Texture& operator=(Texture&&) noexcept = default;
      [[nodiscard]] auto GetBindlessHandle() const -> ::GLuint64 { return _bindless_handle; }

      [[nodiscard]] auto GetWidth()  const -> std::uint32_t    { return _width; }
      [[nodiscard]] auto GetHeight() const -> std::uint32_t    { return _height; }
      [[nodiscard]] auto GetName()   const -> std::string_view { return _name; }
  };
  using Texture2D = Texture<TextureType::Texture2D>;

}
