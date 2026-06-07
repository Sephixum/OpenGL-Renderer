#pragma once

#include "Utils/UniqueHandle.hpp"
#include <glad.h>

namespace glr
{

  class IGLResource
  {
    protected:
      UniqueHandle<::GLuint> _id;

    public:
      IGLResource() = default;
      virtual ~IGLResource() = default;

      IGLResource(IGLResource&&) = default;
      auto operator=(IGLResource&&) -> IGLResource&  = default;
      IGLResource(IGLResource const&) = delete;
      auto operator=(IGLResource const&) -> IGLResource& = delete;

      [[nodiscard]] auto GetID() const -> GLuint { return _id; };
  };

}
