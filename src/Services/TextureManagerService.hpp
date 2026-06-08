#pragma once

#include "IService.hpp"
#include "Graphics/Texture.hpp"

namespace glr
{

  class TextureManagerService : public IService
  {
    std::unordered_map<std::string, Texture2D> _2d = {};

    public:
      virtual auto OnInit()     -> void override;
      virtual auto OnUpdate()   -> void override;
      virtual auto OnShutdown() -> void override;

      auto AddTexture2D(Texture2DCreateInfo const& info, std::string_view name) -> void;

      [[nodiscard]] auto GetTexture2D(std::string_view name)       -> Texture2D&;
      [[nodiscard]] auto TeyGetTexture2D(std::string_view name)    -> Texture2D*;
      [[nodiscard]] auto IsTextureLoaded(std::string_view name) -> bool;
      [[nodiscard]] auto GetAllTags() -> std::vector<std::string>
      {
        auto vec = std::vector<std::string>(_2d.size());
        for (auto const& [name , _] : _2d)
        {
          vec.push_back(name);
        }
        return vec;
      }

  };

}
