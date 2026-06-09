#pragma once

#include "Services/IService.hpp"
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <flat_map>
#include <unordered_map>

namespace glr
{

  struct Scene
  {
    entt::registry registry = {};

    [[nodiscard]] auto GetActiveCamera()                                      -> entt::entity;
    [[nodiscard]] auto CreateEntity(std::string_view name = "Unnamed Entity") -> entt::entity;
  };


  class SceneManagerService : public IService
  {
    std::unordered_map<std::string, Scene> _scenes       = {};
    Scene*                                 _active_scene = nullptr;

    public:
      virtual auto OnInit()     -> void override;
      virtual auto OnUpdate()   -> void override;
      virtual auto OnShutdown() -> void override;

      [[nodiscard]] auto GetActiveScene()     -> Scene&;
      auto CreateScene(std::string_view name) -> Scene&;
      auto SetActiveScene(Scene& scene) -> void;

      [[nodiscard]] auto GetScene(std::string_view name)    -> Scene&;
      [[nodiscard]] auto TryGetScene(std::string_view name) -> Scene*;
  };

}
