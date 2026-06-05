#pragma once

#include "IService.hpp"
#include "Graphics/ShaderProgram.hpp"
#include <unordered_map>
#include <string>

namespace glr 
{

  class ResourceLoaderService : public IService 
  {
    auto LoadModelFromFile(std::filesystem::path const& path, std::string const& name) -> void;
    auto LoadAllAssetModels() -> void;

    public:
      auto OnInit()     -> void override;
      auto OnUpdate()   -> void override;
      auto OnShutdown() -> void override;

      auto LoadModel(std::filesystem::path const& absolute_path_dir)  -> void;
  };

}
