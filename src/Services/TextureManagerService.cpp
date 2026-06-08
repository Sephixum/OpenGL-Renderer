#include "TextureManagerService.hpp"
#include "Graphics/Texture.hpp"
#include "Utils/Error.hpp"
#include "Utils/Log.hpp"
#include <ranges>

namespace glr
{

  auto TextureManagerService::OnInit() -> void 
  {

  }

  auto TextureManagerService::OnUpdate() -> void 
  {

  }

  auto TextureManagerService::OnShutdown() -> void 
  {

  }

  auto TextureManagerService::AddTexture2D(Texture2DCreateInfo const& info, std::string_view name) -> void
  {
    _2d.emplace(std::string{name}, Texture2D(info, name));
  }

  auto TextureManagerService::GetTexture2D(std::string_view name) -> Texture2D&
  {
    auto it = _2d.find(std::string{name});
    Expect(it != _2d.end(), "Texture {} is not loaded", name);
    return it->second;
  }

  auto TextureManagerService::TeyGetTexture2D(std::string_view name) -> Texture2D*
  {
    auto it = _2d.find(std::string{name});
    if (it == _2d.end())
    {
      return nullptr;
    }
    return &(it->second);
  }

  auto TextureManagerService::IsTextureLoaded(std::string_view name) -> bool
  {
    return _2d.contains(std::string{name});
  }

}
