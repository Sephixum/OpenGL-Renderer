#pragma once

#include "Sampler.hpp"

namespace glr::SamplerLibrary 
{

  inline const Sampler& ModelMipmapRepeat() {
    static const Sampler s(
      SamplerCreateInfo{
          .min_filter = MinFilterType::LinearMipMapLinear,
          .mag_filter = MagFilterType::Linear,
          .wrap_s     = WrapMode::Repeat,
          .wrap_t     = WrapMode::Repeat
          // NO anisotropy_samples – stays std::nullopt
      },
      "ModelMipmapRepeat"
    );
    return s;
  }

  // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  // High quality model (trilinear + 8x anisotropic)
  // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  inline const Sampler& HighQualityModel() {
    static const Sampler s(
      SamplerCreateInfo{
          .min_filter = MinFilterType::LinearMipMapLinear,
          .mag_filter = MagFilterType::Linear,
          .wrap_s     = WrapMode::Repeat,
          .wrap_t     = WrapMode::Repeat,
          .anisotropy_samples = 8.0f
      },
      "HighQualityModel"
    );
    return s;
  }

  // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  // Low quality model (bilinear, no mip, no anisotropy)
  // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  inline const Sampler& LowQualityModel() {
    static const Sampler s(
      SamplerCreateInfo{
          .min_filter = MinFilterType::Linear,
          .mag_filter = MagFilterType::Linear,
          .wrap_s     = WrapMode::Repeat,
          .wrap_t     = WrapMode::Repeat
          // no anisotropy
      },
      "LowQualityModel"
    );
    return s;
  }

  // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  // Framebuffer sampler (bilinear, clamp, no mip)
  // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  inline const Sampler& Framebuffer() {
    static const Sampler s(
        SamplerCreateInfo{
            .min_filter = MinFilterType::Linear,
            .mag_filter = MagFilterType::Linear,
            .wrap_s     = WrapMode::ClampToEdge,
            .wrap_t     = WrapMode::ClampToEdge
        },
        "Framebuffer"
    );
    return s;
  }

  // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  // Model mipmap with different anisotropy levels
  // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  inline const Sampler& ModelMipmapAniso2x() {
    static const Sampler s(
        SamplerCreateInfo{
            .min_filter = MinFilterType::LinearMipMapLinear,
            .mag_filter = MagFilterType::Linear,
            .wrap_s     = WrapMode::Repeat,
            .wrap_t     = WrapMode::Repeat,
            .anisotropy_samples = 2.0f
        },
        "ModelMipmapAniso2x"
    );
    return s;
  }

  inline const Sampler& ModelMipmapAniso4x() {
    static const Sampler s(
        SamplerCreateInfo{
            .min_filter = MinFilterType::LinearMipMapLinear,
            .mag_filter = MagFilterType::Linear,
            .wrap_s     = WrapMode::Repeat,
            .wrap_t     = WrapMode::Repeat,
            .anisotropy_samples = 4.0f
        },
        "ModelMipmapAniso4x"
    );
    return s;
  }

  inline const Sampler& ModelMipmapAniso8x() {
    static const Sampler s(
        SamplerCreateInfo{
            .min_filter = MinFilterType::LinearMipMapLinear,
            .mag_filter = MagFilterType::Linear,
            .wrap_s     = WrapMode::Repeat,
            .wrap_t     = WrapMode::Repeat,
            .anisotropy_samples = 8.0f
        },
        "ModelMipmapAniso8x"
    );
      return s;
  }

  inline const Sampler& ModelMipmapAniso16x() {
    static const Sampler s(
        SamplerCreateInfo{
            .min_filter = MinFilterType::LinearMipMapLinear,
            .mag_filter = MagFilterType::Linear,
            .wrap_s     = WrapMode::Repeat,
            .wrap_t     = WrapMode::Repeat,
            .anisotropy_samples = 16.0f
        },
        "ModelMipmapAniso16x"
    );
    return s;
  }

} // namespace glr::SamplerLibrary
