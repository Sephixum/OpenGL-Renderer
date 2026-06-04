#pragma once

#include "Utils/UniqueHandle.hpp"
#include "Buffer.hpp"
#include <glad.h>
#include <cstdint>

namespace glr
{

  template <typename T>
  class UniformBuffer 
  {
    UniqueHandle<::GLuint> _id;
    std::string            _name;
    std::uint32_t          _binding;

    public:
      UniformBuffer(std::string_view name = "Unknown UniformBuffer")
        : _id(0u, [](auto e){ ::glUnmapNamedBuffer(e); ::glDeleteBuffers(1, &e);})
        , _name{name}
        , _binding{0u}
      {
        ::glCreateBuffers(1, &_id);
        ::glNamedBufferStorage(
          _id,
          sizeof(T),
          nullptr,
          GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
        );
        ::glObjectLabel(GL_BUFFER, _id, name.length(), name.data());
      }
  };

}
