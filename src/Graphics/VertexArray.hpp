#pragma once

#include <glad.h>
#include <cstdint>
#include "Buffer.hpp"
#include "Utils/UniqueHandle.hpp"

namespace glr 
{

  class VertexArray
  {
    UniqueHandle<::GLuint> _id;

    public:
      class SettingBuilder;

    public:
      VertexArray(std::string_view name = "Unknown VertexArray")
        : _id(0u, [](auto e){::glDeleteVertexArrays(1, &e);})
      {
        ::glCreateVertexArrays(1, &_id);
        ::glObjectLabel(GL_VERTEX_ARRAY, _id, name.length(), name.data());
      }

      auto BuildSettings() -> SettingBuilder;

      auto Activate() -> void;
  };
  
  class VertexArray::SettingBuilder
  {
    friend VertexArray;

    VertexArray& m_instance;

    SettingBuilder(VertexArray& instance) : m_instance{instance}
    {
    }

    public:
      template <BufferType Type>
        requires (Type != BufferType::ShaderStorage)
             and (Type != BufferType::TransformFeedback)
             and (Type != BufferType::Uniform)
      auto BindAs(this SettingBuilder&& self, IBuffer& buffer) -> SettingBuilder&&
      {
        buffer.BindAs<Type>();
        return self;
      }

      template <BufferType Type>
        requires (Type == BufferType::ShaderStorage)
              or (Type == BufferType::TransformFeedback)
              or (Type == BufferType::Uniform)
      auto BindAs(this SettingBuilder&& self, IBuffer& buffer, std::uint32_t index) -> SettingBuilder&&
      {
        buffer.BindAs<Type>(index);
        return self;
      }

      auto Apply(this SettingBuilder&& self) -> VertexArray&
      {
        ::glBindVertexArray(0);
        return self.m_instance;
      }
  };

  inline auto VertexArray::BuildSettings() -> SettingBuilder
  {
    ::glBindVertexArray(_id);
    return {*this};
  }

  inline auto VertexArray::Activate() -> void
  {
    ::glBindVertexArray(_id);
  }

}
