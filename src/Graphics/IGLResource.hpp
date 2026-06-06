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
      [[nodiscard]] auto GetID() const -> GLuint { return _id; };
  };

}
