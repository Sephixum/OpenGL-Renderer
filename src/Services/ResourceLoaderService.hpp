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
    auto LoadModelTextures(
        std::filesystem::path const& model_file_path,
        aiScene               const* scene
      ) -> void;

    public:
      auto OnInit()     -> void override;
      auto OnUpdate()   -> void override;
      auto OnShutdown() -> void override;

      auto LoadModel(std::filesystem::path const& absolute_path_dir)                     -> void;
      auto LoadModelFromFile(std::filesystem::path const& path, std::string const& name) -> void;

      auto LoadTexture(std::filesystem::path const& absolute_path_dir)                     -> void;
      auto LoadTextureCustomTag(std::filesystem::path const& absolute_path_dir,
                                                                  std::string const& tag)  -> void;
      auto LoadTextureFromFile(std::filesystem::path const& path, std::string const& name) -> void;
      
  };

}
