#pragma once

#include "IService.hpp"
#include "Graphics/ShaderProgram.hpp"
#include <assimp/scene.h>
#include <unordered_map>
#include <string>

namespace glr 
{

  class ResourceLoaderService : public IService 
  {
    auto LoadAllAssetModels() -> void;
    auto LoadAllAssetShaders()     -> void;

    auto LoadModelFromFile(std::filesystem::path const& abs_path,std::string const& name)   -> void;
    auto LoadTextureFromFile(std::filesystem::path const& abs_path, std::string const& tag) -> void;

    struct GlobalMaterial 
    {
      std::string albedo_tag;
      std::string normal_tag;
      std::string roughness_tag;
      std::string metalic_tag;
    };

    std::vector<GlobalMaterial> _global_materials;

    public:
      auto OnInit()     -> void override;
      auto OnUpdate()   -> void override {}
      auto OnShutdown() -> void override {}

      auto LoadModel(std::filesystem::path const& abs_model)                               -> void;
      auto LoadModelTagged(std::filesystem::path const& abs_path, std::string const& name) -> void;

      auto LoadTexture(std::filesystem::path const& abs_path)                               -> void;
      auto LoadTextureTagged(std::filesystem::path const& abs_path, std::string const& name) -> void;

      auto LoadShader(std::filesystem::path const& abs_path)                                -> void;
      auto LoadShaderTagged(std::filesystem::path const& abs_path, std::string const& name) -> void;

      auto GetGlobalMaterial() -> std::span<GlobalMaterial>;
  };

}
