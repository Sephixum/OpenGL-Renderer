#pragma once

#include "Services/IService.hpp"
#include <entt/entity/fwd.hpp>
#include <unordered_map>

namespace glr
{

  using Scene = entt::registry;

  struct SceneRef
  {
    std::string_view name;
    Scene&           scene;
  };

  class SceneManagerService : public IService
  {
    std::unordered_map<std::string, Scene> _scenes       = {};
    std::pair<std::string_view, Scene*>   _active_scene = {};

    public:
      virtual auto OnInit()     -> void override;
      virtual auto OnUpdate()   -> void override;
      virtual auto OnShutdown() -> void override;

      [[nodiscard]] auto GetActiveScene()     -> SceneRef;
      auto CreateScene(std::string_view name) -> SceneRef;
      auto SetActiveScene(SceneRef scene) -> void;
  };

}
