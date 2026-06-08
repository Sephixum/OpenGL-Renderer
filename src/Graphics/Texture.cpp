#include "Texture.hpp"
#include "DataTypes.hpp"
#include "Sampler.hpp"
#include "Utils/Log.hpp"

#include <cmath>
#include <glad.h>
#include <utility>

  ///////////////// (1)
  // "
  //   The effective target is not TEXTURE_1D_ARRAY or PROXY_TEXTURE_-
  //   1D_ARRAY, and levels is greater than [log2(max(width, height))] + 1
  // "
  // [log2(max(width, height))] + 1 -> basically "floor(log2(max(width, height))) + 1"
  // Scetion 8.19, Page 278, OpenGL 4.6 core profile specs by khronos group from may 5, 2022
  //
  // - Discovery : 6/6/26

namespace
{

  struct TextureUploadInfo
  {
    GLenum pixel_layout;
    GLenum data_type;
  };

  [[nodiscard]] constexpr auto UploadInfo(glr::TextureFormatType f) -> TextureUploadInfo
  {
    switch (f)
    {
      using enum glr::TextureFormatType;
      case R8unorm         : return {GL_RED , GL_UNSIGNED_BYTE};
      case Rg8unorm        : return {GL_RG  , GL_UNSIGNED_BYTE};
      case Rgb8unorm       : return {GL_RGB , GL_UNSIGNED_BYTE};
      case Rgba8unorm      : return {GL_RGBA, GL_UNSIGNED_BYTE};

      case Rgba16f         : return {GL_RGBA, GL_HALF_FLOAT};
      case Rgb16f          : return {GL_RGB , GL_HALF_FLOAT};
      case R16f            : return {GL_RED , GL_HALF_FLOAT};

      case Rgba32f         : return {GL_RGBA           , GL_FLOAT};
      case Depth24stencil8 : return {GL_DEPTH_STENCIL  , GL_UNSIGNED_INT_24_8};
      case Depth32f        : return {GL_DEPTH_COMPONENT, GL_FLOAT };
    }
    std::unreachable();
  }

}

namespace glr
{

  Texture2D::Texture(Texture2DCreateInfo const& info, std::string_view name)
    : _width{info.width}
    , _height{info.height}
    , _name{name}
  {
    _id              = {0u , [](auto e){::glDeleteTextures(1, &e);}};
    _bindless_handle = {0zu, [](auto e){::glMakeTextureHandleNonResidentARB(e);}};
    ::glCreateTextures(GL_TEXTURE_2D, 1, &_id);
    ::glObjectLabel(GL_TEXTURE, _id, name.length(), name.data());

    // (1)
    std::size_t const max_mipmap_levels = 1 + std::floor(std::log2(std::max(info.width, info.height)));
    bool        const has_data          = info.data.has_value();
    auto              mipmap_levels     = 0zu;

    if (has_data and info.mipmap_levels > 0)
    {
       mipmap_levels = std::min<std::size_t>(info.mipmap_levels, max_mipmap_levels);
    }
    else
    {
      mipmap_levels = max_mipmap_levels;
    }

    ::glTextureStorage2D(_id, mipmap_levels, std::to_underlying(info.format), info.width, info.height);

    if (has_data)
    {
      auto const& pixels_vec                 = info.data.value();
      auto const  [pixel_layout, pixel_type] = UploadInfo(info.format);
      ::glTextureSubImage2D(
          _id, 
          0, 
          0, 0,
          info.width, info.height,
          pixel_layout,
          pixel_type,
          pixels_vec.data()
          );

      if (mipmap_levels > 1)
      {
        ::glGenerateTextureMipmap(_id);
      }
    }

    _bindless_handle.Reset(::glGetTextureSamplerHandleARB(_id, info.sampler.GetID()));
    ::glMakeTextureHandleResidentARB(_bindless_handle);
  }

}
