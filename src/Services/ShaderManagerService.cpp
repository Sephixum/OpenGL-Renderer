#include "ShaderManagerService.hpp"

namespace glr
{

  namespace fs = std::filesystem;

  auto ShaderManagerService::GetShaderPath(std::string const& name, std::string const& extension) const -> fs::path
  {
    return fs::current_path() / "assets" / "shaders" / name / (name + extension);
  }

  auto ShaderManagerService::LoadShader(std::string const& name) -> void 
  {
    auto vert_path = GetShaderPath(name, ".vert");
    auto frag_path = GetShaderPath(name, ".frag");
    auto comp_path = GetShaderPath(name, ".comp");

    auto has_vert = fs::exists(vert_path);
    auto has_frag = fs::exists(frag_path);
    auto has_comp = fs::exists(comp_path);

    Expect(has_vert or has_comp or has_frag, "There are no shaders in {}/", name);

    if (has_comp)
    {
      _compute_shaders.emplace(name, ShaderProgram<ShaderStage::Compute>(comp_path, name + ".comp"));
    }

    if (has_vert)
    {
      _vertex_shaders.emplace(name, ShaderProgram<ShaderStage::Vertex>(vert_path, name + ".vert"));
    }

    if (has_frag)
    {
      _fragment_shaders.emplace(name, ShaderProgram<ShaderStage::Fragment>(frag_path, name + ".frag"));
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
