#pragma once

#include "Components/include/RelationshipComponent.hpp"
#include "Services/IService.hpp"
#include <concepts>
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <flat_map>
#include <glm/ext/vector_float3.hpp>
#include <unordered_map>

namespace glr
{


  struct CameraCreateInfo
  {
    std::string name = "Camera";

    glm::vec3 position = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 rotation = glm::vec3(0.0f);

    bool primary = false;
  };


  struct Scene
  {
    entt::registry registry = {};

    [[nodiscard]] auto GetActiveCamera()                                      -> entt::entity;
    [[nodiscard]] auto CreateEntity(std::string_view name = "Unnamed Entity") -> entt::entity;

    [[nodiscard]] auto CreateChild(entt::entity parent, std::string_view name = {}) -> entt::entity;
    auto SetParent(entt::entity child, entt::entity parent)                         -> void;
    auto DestroyEntity(entt::entity entity)                                         -> void;
    auto ForEachChild(entt::entity parent, std::invocable<entt::entity> auto&& fn)  -> void;
    [[nodiscard]] auto CreateCamera(CameraCreateInfo const& info = {})              -> entt::entity;
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


  auto Scene::ForEachChild(entt::entity parent, std::invocable<entt::entity> auto&& fn) -> void
  {
    auto child = registry.get<Component::Relationship>(parent).first_child;

    while (child != entt::null)
    {
      fn(child);

      child =
        registry.get<Component::Relationship>(child)
          .next_sibling;
    }
  }

}
