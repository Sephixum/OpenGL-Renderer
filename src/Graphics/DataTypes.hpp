#pragma once

#include <glad.h>

namespace glr
{

  enum struct DataType
  {
    Int       = GL_INT,
    Uint      = GL_UNSIGNED_INT,
    Float     = GL_FLOAT,
    HalfFloat = GL_HALF_FLOAT,
    Byte      = GL_BYTE,
    Ubyte     = GL_UNSIGNED_BYTE,
    Uint248   = GL_UNSIGNED_INT_24_8
  };

}
