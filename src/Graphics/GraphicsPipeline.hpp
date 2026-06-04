#pragma once

#include <glad.h>
#include "ShaderProgram.hpp"
#include "Utils/UniqueHandle.hpp"
#include <string_view>

namespace glr
{

  class GraphicsPipeline
  {
    UniqueHandle<::GLuint> _id;
    bool                   _validated = false;

    auto ValidateProgramLinking() -> void
    {
      auto success_flag = ::GLint{};
      auto info_log     = std::string(512, '\0');
      ::glValidateProgramPipeline(_id);
      ::glGetProgramPipelineiv(_id, GL_VALIDATE_STATUS, &success_flag);
      if (not success_flag)
      {
        ::glGetProgramPipelineInfoLog(_id, info_log.size(), nullptr, info_log.data());
        throw Exception{"Shader program linking failed. reason : {}", info_log};
      }
    }

    public:
      GraphicsPipeline(std::string_view name = "Unknown GraphicsPipeline")
        : _id(0u, [](auto e){::glDeleteProgramPipelines(1, &e);})
      {
        ::glCreateProgramPipelines(1, &_id);
        ::glObjectLabel(GL_PROGRAM_PIPELINE, _id, name.length(), name.data());
      }

      auto Activate() -> void
      {
        if (not _validated)
        {
          ValidateProgramLinking();
          _validated = true;
        }
        ::glBindProgramPipeline(_id);
      }

      auto SetVertexShader(ShaderProgram<ShaderStage::Vertex> const& shader) -> GraphicsPipeline&
      {
        ::glUseProgramStages(_id, GL_VERTEX_SHADER_BIT, shader.GetID());
        _validated = false;
        return *this;
      }
      
      auto SetFragmentShader(ShaderProgram<ShaderStage::Fragment> const& shader) -> GraphicsPipeline&
      {
        ::glUseProgramStages(_id, GL_FRAGMENT_SHADER_BIT, shader.GetID());
        _validated = false;
        return *this;
      }
  };

}
