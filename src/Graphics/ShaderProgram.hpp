#pragma once

#include <glad.h>

#include "Graphics/IGLResource.hpp"
#include "Utils/Exception.hpp"
#include "Utils/Formatter.hpp"
#include "Utils/UniqueHandle.hpp"

#include <fstream>
#include <iterator>
#include <string_view>
#include <utility>
#include <filesystem>

namespace glr 
{

  namespace stdfs = std::filesystem;
  
  enum struct ShaderStage
  {
    Vertex   = GL_VERTEX_SHADER_BIT,
    Fragment = GL_FRAGMENT_SHADER_BIT,
    Compute  = GL_COMPUTE_SHADER_BIT
  };
  
  template <ShaderStage Stage>
  class ShaderProgram : public IGLResource
  {
    std::string _name;

    static auto GetShaderSource(stdfs::path const& path) -> std::string
    {
      auto file = std::ifstream{path};
      if (not file)
      {
        throw Exception{"Failed to load shader {}", path.filename().string()};
      }

      return std::string{std::istreambuf_iterator{file}, {}};
    }

    auto ValidateProgramLinking() -> void
    {
      auto success_flag = ::GLint{};
      auto info_log     = std::string(512, '\0');
      ::glGetProgramiv(_id, GL_LINK_STATUS, &success_flag);
      if (not success_flag)
      {
        ::glGetProgramInfoLog(_id, info_log.size(), nullptr, info_log.data());
        throw Exception{"Shader program {} linking failed. reason : {}", _name, info_log};
      }
    }

    public:
      ShaderProgram(stdfs::path const& path, std::string_view name = "Unknown ShaderProgram")
        : _name{name}
      {

        _id = {0u, [](auto e){::glDeleteProgram(e);}};
        static constexpr auto s_shader_level_stage =  [] consteval
        {
          switch (Stage) 
          {
            case ShaderStage::Vertex:   return GL_VERTEX_SHADER;
            case ShaderStage::Fragment: return GL_FRAGMENT_SHADER;
            case ShaderStage::Compute:  return GL_COMPUTE_SHADER;
          }
          std::unreachable();
        }();

        auto        source     = GetShaderSource(path);
        char const* source_ptr = source.c_str();

        _id.Reset(::glCreateShaderProgramv(s_shader_level_stage, 1, &source_ptr));
        ValidateProgramLinking();
        ::glObjectLabel(GL_PROGRAM, _id, name.length(), name.data());
      }
  };

  [[nodiscard]] constexpr auto to_string(ShaderStage s)
  {
    switch (s)
    {
      using enum ShaderStage;
      case Vertex  : return "Vertex";
      case Fragment: return "Fragment";
      case Compute : return "Compute";
    }
    std::unreachable();
  }
  static_assert(CanFormat<ShaderStage>);

  [[nodiscard]] constexpr auto to_opengl(ShaderStage s)
  {
    switch (s)
    {
      case ShaderStage::Vertex:   return GL_VERTEX_SHADER_BIT;
      case ShaderStage::Fragment: return GL_FRAGMENT_SHADER_BIT;
      case ShaderStage::Compute:  return GL_FRAGMENT_SHADER_BIT;
    }
    std::unreachable();
  }

}
