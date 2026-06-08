#pragma once

#include "Graphics/ShaderProgram.hpp"
#include "Services/IService.hpp"
#include <string>
#include <unordered_map>

namespace glr
{

  class ShaderManagerService : public IService
  {
    std::unordered_map<std::string, ShaderProgram<ShaderStage::Vertex>>   _vertex_shaders;
    std::unordered_map<std::string, ShaderProgram<ShaderStage::Fragment>> _fragment_shaders;
    std::unordered_map<std::string, ShaderProgram<ShaderStage::Compute>>  _compute_shaders;

    auto GetShaderPath(const std::string& name, const std::string& extension) const -> std::filesystem::path ;

    public:
      virtual auto OnInit()     -> void override;
      virtual auto OnUpdate()   -> void override;
      virtual auto OnShutdown() -> void override;
      
      auto LoadShader(std::filesystem::path const& abs_path) -> void;

      [[nodiscard]] auto GetVertexShader(std::string const& name)   const -> ShaderProgram<ShaderStage::Vertex> const&;
      [[nodiscard]] auto GetFragmentShader(std::string const& name) const -> ShaderProgram<ShaderStage::Fragment> const&;
      [[nodiscard]] auto GetComputeShader(std::string const& name)  const -> ShaderProgram<ShaderStage::Compute> const&;
      [[nodiscard]] auto IsShaderLoaded(std::string const& name)    const -> bool;
      
  };

}
