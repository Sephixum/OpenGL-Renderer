#include "SceneManagerService.hpp"
#include "Components/Components.hpp"
#include "Graphics/Model.hpp"
#include "Utils/Error.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace helper
{

  [[nodiscard]] auto BuildModelNode(glr::Scene& scene, glr::NodeData const& node, std::string_view model_name, entt::entity parent) -> entt::entity;

}

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

  auto SceneManagerService::GetScene(std::string_view name) -> Scene&
  {
    auto iter = _scenes.find(std::string{name});
    Expect(iter != _scenes.end(), "Scene {} not found !", name);
    return iter->second;
  }

  auto SceneManagerService::TryGetScene(std::string_view name) -> Scene*
  {
    auto iter = _scenes.find(std::string{name});

    if (iter == _scenes.end())
    {
      return nullptr;
    }

    return &(iter->second);
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

  auto Scene::CreateEntity(std::string_view name) -> entt::entity
  {
    auto e = registry.create();

    registry.emplace<Component::Name>(e, std::string{name});
    registry.emplace<Component::Transform>(e);
    registry.emplace<Component::GlobalTransform>(e);
    registry.emplace<Component::Relationship>(e);

    return e;
  }

  auto Scene::CreateCamera(CameraCreateInfo const& info) -> entt::entity
  {
    auto entity = CreateEntity(info.name);

    auto& transform = registry.get<Component::Transform>(entity);

    transform.position = info.position;
    transform.rotation = glm::quat(glm::radians(info.rotation));

    registry.emplace<Component::Camera>(entity);

    auto& camera = registry.get<Component::Camera>(entity);

    camera.primary = info.primary;

    return entity;
  }


  auto Scene::CreateChild(entt::entity parent, std::string_view name) -> entt::entity
  {
    auto child = CreateEntity(name);
    SetParent(child, parent);
    return child;
  }

  auto Scene::SetParent(entt::entity child, entt::entity parent) -> void
  {
    auto& child_rel = registry.get<Component::Relationship>(child);

    // Detach from previous parent
    if (child_rel.parent != entt::null)
    {
      auto& old_parent_rel = registry.get<Component::Relationship>(child_rel.parent);

      if (old_parent_rel.first_child == child)
      {
        old_parent_rel.first_child = child_rel.next_sibling;
      }
      else
      {
        auto current = old_parent_rel.first_child;

        while (current != entt::null)
        {
          auto& rel = registry.get<Component::Relationship>(current);

          if (rel.next_sibling == child)
          {
            rel.next_sibling = child_rel.next_sibling;
            break;
          }

          current = rel.next_sibling;
        }
      }
    }

    // Attach to new parent
    child_rel.parent       = parent;
    child_rel.next_sibling = entt::null;

    if (parent == entt::null)
    {
      return;
    }

    auto& parent_rel = registry.get<Component::Relationship>(parent);

    child_rel.next_sibling = parent_rel.first_child;
    parent_rel.first_child = child;
  }

  auto Scene::DestroyEntity(entt::entity entity) -> void
  {
    auto const rel = registry.get<Component::Relationship>(entity);

    // Destroy children first
    auto child = rel.first_child;

    while (child != entt::null)
    {
      auto next = registry.get<Component::Relationship>(child).next_sibling;

      DestroyEntity(child);

      child = next;
    }

    // Remove from parent
    if (rel.parent != entt::null)
    {
      auto& parent_rel = registry.get<Component::Relationship>(rel.parent);

      if (parent_rel.first_child == entity)
      {
        parent_rel.first_child = rel.next_sibling;
      }
      else
      {
        auto current = parent_rel.first_child;

        while (current != entt::null)
        {
          auto& current_rel = registry.get<Component::Relationship>(current);

          if (current_rel.next_sibling == entity)
          {
            current_rel.next_sibling = rel.next_sibling;
            break;
          }

          current = current_rel.next_sibling;
        }
      }
    }

    registry.destroy(entity);
  }

}



auto helper::BuildModelNode(glr::Scene& scene, glr::NodeData const& node, std::string_view model_name, entt::entity parent) -> entt::entity
{
  auto entity =
    parent == entt::null
    ? scene.CreateEntity(node.name)
    : scene.CreateChild(parent, node.name);

  auto& transform = scene.registry.get<glr::Component::Transform>(entity);

  auto scale       =  glm::vec3{};
  auto rotation    =  glm::quat{};
  auto translation =  glm::vec3{};
  auto skew        =  glm::vec3{};
  auto perspective =  glm::vec4{};

  glm::decompose(
      node.local_transform,
      scale,
      rotation,
      translation,
      skew,
      perspective);

  transform.position = translation;
  transform.rotation = rotation;
  transform.scale    = scale;

  // Create mesh entities
  for (glr::u32 mesh_index : node.mesh_indices)
  {
    auto mesh_entity = scene.CreateChild(entity, std::format("Mesh_{}", node.name));

    scene.registry.emplace<glr::Component::MeshRenderer>(
        mesh_entity,
        mesh_index,
        std::string(model_name));
  }

  // Recurse children
  for (auto const& child : node.children)
  {
    [[maybe_unused]] auto e = BuildModelNode(
                                      scene,
                                      child,
                                      model_name,
                                      entity);
  }

  return entity;
}


