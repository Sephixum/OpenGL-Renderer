#include "Sampler.hpp"
#include "Utils/Utils.hpp"

#include <utility>
#include <algorithm>

namespace glr
{


  Sampler::Sampler(SamplerCreateInfo const& info, std::string_view name)
  {
    // Used to clamp invalid values
    static float const max_anisotropy = []
    {
      auto max = f32{};
      ::glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &max);
      return max;
    }();

    _id = {0u, [](auto e){::glDeleteSamplers(1, &e);}};

    ::glCreateSamplers(1, &_id);
    ::glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, std::to_underlying(info.min_filter));
    ::glSamplerParameteri(_id, GL_TEXTURE_MAG_FILTER, std::to_underlying(info.mag_filter));
    ::glSamplerParameteri(_id, GL_TEXTURE_WRAP_S    , std::to_underlying(info.wrap_s));
    ::glSamplerParameteri(_id, GL_TEXTURE_WRAP_T    , std::to_underlying(info.wrap_t));

    if (info.anisotropy_samples.has_value())
    {
      ::glSamplerParameterf(
          _id,
          GL_TEXTURE_MAX_ANISOTROPY,
          std::clamp(info.anisotropy_samples.value(), 1.0f, max_anisotropy)
      );
    }

    if (info.compare_func.has_value())
    {
      ::glSamplerParameteri(_id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
      ::glSamplerParameteri(_id, GL_TEXTURE_COMPARE_FUNC, std::to_underlying(info.compare_func.value()));
    }
    else
    {
      ::glSamplerParameteri(_id, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    }

    ::glObjectLabel(GL_SAMPLER, _id, name.length(), name.data());
  }

}
