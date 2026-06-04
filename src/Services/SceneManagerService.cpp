#include "SceneManagerService.hpp"
#include "Components/Components.hpp"

#include <entt/entt.hpp>

namespace glr
{

  auto SceneManagerService::OnInit()     -> void {}

  auto SceneManagerService::OnUpdate()   -> void {}

  auto SceneManagerService::OnShutdown() -> void {}

  auto SceneManagerService::GetActiveScene() -> Scene&
  {
    if (_active_scene == nullptr)
    {
      if (_scenes.size() == 0)
      {
        return CreateScene("Default Scene");
      }
      else
      {
        return _scenes.begin()->second;
      }
    }
    else return *_active_scene;
  }

  auto SceneManagerService::CreateScene(std::string_view name) -> Scene&
  {
    auto name_str = std::string{name};
    _scenes[name_str] = Scene{};
    return _scenes[name_str];
  }

  auto SceneManagerService::SetActiveScene(Scene& scene) -> void
  {
    _active_scene = &scene;
  }

  auto Scene::GetActiveCamera() -> entt::entity
  {

    auto view = registry.view<Component::Camera,
                              Component::ActiveCamera,
                              Component::Transform,
                              Component::Projection>();

    if (view.size_hint() == 0)
    {
      return entt::null;
    }

    return *view.begin();
  }

}
