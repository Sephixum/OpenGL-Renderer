#include "ShaderManagerService.hpp"

#include "Utils/Error.hpp"

namespace glr
{

  namespace fs = std::filesystem;

  auto ShaderManagerService::LoadShader(fs::path const& abs_path) -> void 
  {

    auto extension = abs_path.extension().string();
    auto name      = abs_path.stem().string();

    log::Info("loading {}", abs_path.string());

    if (extension == ".comp")
    {
      _compute_shaders.insert_or_assign(name, ShaderProgram<ShaderStage::Compute>(abs_path, name + ".comp"));
    }
    else if (extension == ".vert")
    {
      _vertex_shaders.insert_or_assign(name, ShaderProgram<ShaderStage::Vertex>(abs_path, name + ".vert"));
    }
    else if (extension == ".frag")
    {
      _fragment_shaders.insert_or_assign(name, ShaderProgram<ShaderStage::Fragment>(abs_path, name + ".frag"));
    }
    else
    {
      return;
    }
  }

  auto ShaderManagerService::GetVertexShader(std::string const& name)   const -> ShaderProgram<ShaderStage::Vertex> const& 
  {
    auto it = _vertex_shaders.find(name);
    Expect(it != _vertex_shaders.end(), "Vertex shader \"{}\" not loaded", name);
    return it->second;
  }

  auto ShaderManagerService::GetFragmentShader(std::string const& name) const -> ShaderProgram<ShaderStage::Fragment> const& 
  {
    auto it = _fragment_shaders.find(name);
    Expect(it != _fragment_shaders.end(), "Fragment shader \"{}\" not loaded", name);
    return it->second;
  }

  auto ShaderManagerService::GetComputeShader(std::string const& name)  const -> ShaderProgram<ShaderStage::Compute> const& 
  {
    auto it = _compute_shaders.find(name);
    Expect(it != _compute_shaders.end(), "Compute shader \"{}\" not loaded", name);
    return it->second;
  }

  auto ShaderManagerService::IsShaderLoaded(std::string const& name) const -> bool 
  {
    return _vertex_shaders.count(name) || _fragment_shaders.count(name) || _compute_shaders.count(name);
  }

  auto ShaderManagerService::OnInit() -> void 
  {

  }

  auto ShaderManagerService::OnUpdate() -> void 
  {

  }

  auto ShaderManagerService::OnShutdown() -> void 
  {

  }

}
