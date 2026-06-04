#include "SceneManagerService.hpp"

#include <entt/entt.hpp>

namespace glr
{

  auto SceneManagerService::OnInit()     -> void {}

  auto SceneManagerService::OnUpdate()   -> void {}

  auto SceneManagerService::OnShutdown() -> void {}

  auto SceneManagerService::GetActiveScene() -> SceneRef
  {
    if (_active_scene.second == nullptr)
    {
      if (_scenes.size() == 0)
      {
        return CreateScene("Default Scene");
      }
      else
      {
        return {_scenes.begin()->first, _scenes.begin()->second} ;
      }
    }
    else return {_active_scene.first, *_active_scene.second};
  }

  auto SceneManagerService::CreateScene(std::string_view name) -> SceneRef
  {
    auto& scene = *_scenes.emplace(name, Scene{}).first;
    return {scene.first, scene.second};
  }

  auto SceneManagerService::SetActiveScene(SceneRef scene) -> void
  {
    _active_scene = {scene.name, &scene.scene};
  }

}
