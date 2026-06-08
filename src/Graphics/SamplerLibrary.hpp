#pragma once

#include "Sampler.hpp"

namespace glr::SamplerLibrary 
{

  inline const Sampler& ModelMipmapRepeat() {
    static const Sampler s(
      SamplerCreateInfo{
          .min_filter = SamplerMinFilterType::LinearMipMapLinear,
          .mag_filter = SamplerMagFilterType::Linear,
          .wrap_s     = SamplerWrapModeType::Repeat,
          .wrap_t     = SamplerWrapModeType::Repeat
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
          .min_filter = SamplerMinFilterType::LinearMipMapLinear,
          .mag_filter = SamplerMagFilterType::Linear,
          .wrap_s     = SamplerWrapModeType::Repeat,
          .wrap_t     = SamplerWrapModeType::Repeat,
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
          .min_filter = SamplerMinFilterType::Linear,
          .mag_filter = SamplerMagFilterType::Linear,
          .wrap_s     = SamplerWrapModeType::Repeat,
          .wrap_t     = SamplerWrapModeType::Repeat
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
            .min_filter = SamplerMinFilterType::Linear,
            .mag_filter = SamplerMagFilterType::Linear,
            .wrap_s     = SamplerWrapModeType::ClampToEdge,
            .wrap_t     = SamplerWrapModeType::ClampToEdge
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
            .min_filter = SamplerMinFilterType::LinearMipMapLinear,
            .mag_filter = SamplerMagFilterType::Linear,
            .wrap_s     = SamplerWrapModeType::Repeat,
            .wrap_t     = SamplerWrapModeType::Repeat,
            .anisotropy_samples = 2.0f
        },
        "ModelMipmapAniso2x"
    );
    return s;
  }

  inline const Sampler& ModelMipmapAniso4x() {
    static const Sampler s(
        SamplerCreateInfo{
            .min_filter = SamplerMinFilterType::LinearMipMapLinear,
            .mag_filter = SamplerMagFilterType::Linear,
            .wrap_s     = SamplerWrapModeType::Repeat,
            .wrap_t     = SamplerWrapModeType::Repeat,
            .anisotropy_samples = 4.0f
        },
        "ModelMipmapAniso4x"
    );
    return s;
  }

  inline const Sampler& ModelMipmapAniso8x() {
    static const Sampler s(
        SamplerCreateInfo{
            .min_filter = SamplerMinFilterType::LinearMipMapLinear,
            .mag_filter = SamplerMagFilterType::Linear,
            .wrap_s     = SamplerWrapModeType::Repeat,
            .wrap_t     = SamplerWrapModeType::Repeat,
            .anisotropy_samples = 8.0f
        },
        "ModelMipmapAniso8x"
    );
      return s;
  }

  inline const Sampler& ModelMipmapAniso16x() {
    static const Sampler s(
        SamplerCreateInfo{
            .min_filter = SamplerMinFilterType::LinearMipMapLinear,
            .mag_filter = SamplerMagFilterType::Linear,
            .wrap_s     = SamplerWrapModeType::Repeat,
            .wrap_t     = SamplerWrapModeType::Repeat,
            .anisotropy_samples = 16.0f
        },
        "ModelMipmapAniso16x"
    );
    return s;
  }

  inline const Sampler& ShadowMapDepth() 
  {
    static const Sampler s(
      SamplerCreateInfo{
          .min_filter   = SamplerMinFilterType::Linear,
          .mag_filter   = SamplerMagFilterType::Linear,
          .wrap_s       = SamplerWrapModeType::ClampToEdge,
          .wrap_t       = SamplerWrapModeType::ClampToEdge,
          .compare_func = SamplerCompareFuncType::Lequal
      },
      "ShadowMapDepth");
    return s;
  }

  inline const Sampler& FramebufferClamp() {
    static const Sampler s(
        SamplerCreateInfo{
            .min_filter = SamplerMinFilterType::Linear,
            .mag_filter = SamplerMagFilterType::Linear,
            .wrap_s     = SamplerWrapModeType::ClampToEdge,
            .wrap_t     = SamplerWrapModeType::ClampToEdge
        },
        "FramebufferClamp"
    );
    return s;
}

}
