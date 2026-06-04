#include "CameraSystem.hpp"
#include "Components/Components.hpp"
#include "Events/Event.hpp"
#include "Services/ServiceLocator.hpp"
#include "Services/SceneManagerService.hpp"
#include "Utils/Error.hpp"

#include <entt/entity/fwd.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <sys/stat.h>
#include <utility>
#include <GLFW/glfw3.h>

namespace glr
{

  // CameraSystem::CameraSystem()
  // {
  //   _mouse_move_sink   = Application::GetInstance().GetEventBus().sink<Event::MouseMove>().connect<&CameraSystem::OnMouseMove>(this);
  //   _mouse_button_sink = Application::GetInstance().GetEventBus().sink<Event::MouseButton>().connect<&CameraSystem::OnMouseButton>(this);
  //   _mouse_scroll_sink = Application::GetInstance().GetEventBus().sink<Event::MouseScroll>().connect<&CameraSystem::OnMouseScroll>(this);
  // }
  //
  // auto CameraSystem::GetActiveCamera() -> entt::entity
  // {
  //   entt::registry reg;
  //
  //   auto view = reg.view<Component::Camera,
  //                        Component::ActiveCamera,
  //                        Component::Transform,
  //                        Component::Projection>();
  //
  //   Expect(view.size_hint() == 1, "There are more than one active camera !!");
  //   return *view.begin();
  // }
  //
  // auto CameraSystem::OnMouseMove(Event::MouseMove const& e) noexcept -> void 
  // {
  //   entt::registry reg;
  //
  //   auto cam = GetActiveCamera();
  //   auto [camera, transform] = reg.get<Component::Camera, Component::Transform>(cam);
  //   if (camera.mode == Component::Camera::CameraMode::Fly)
  //   {
  //     // simply change state and then
  //     FlyCameraController::OnMouseMove(e, transform, _state);
  //   }
  //   else if (camera.mode == Component::Camera::CameraMode::Orbital)
  //   {
  //     // simply change state and then
  //     OrbitalCameraController::OnMouseMove(e, transform, _state);
  //   }
  //   else
  //   {
  //     std::unreachable();
  //   }
  // }
  //
  // auto CameraSystem::OnMouseButton(Event::MouseButton const& e) noexcept -> void 
  // {
  //   entt::registry reg;
  //
  //   auto cam = GetActiveCamera();
  //   auto [camera, transform] = reg.get<Component::Camera, Component::Transform>(cam);
  //   if (camera.mode == Component::Camera::CameraMode::Fly)
  //   {
  //     // simply change state and then
  //     FlyCameraController::OnMouseButton(e, transform, _state);
  //   }
  //   else if (camera.mode == Component::Camera::CameraMode::Orbital)
  //   {
  //     // simply change state and then
  //     OrbitalCameraController::OnMouseButton(e, transform, _state);
  //   }
  //   else
  //   {
  //     std::unreachable();
  //   }
  // }
  //
  // auto CameraSystem::OnMouseScroll(Event::MouseScroll const& e) noexcept -> void 
  // {
  //   _state.scroll_delta = static_cast<float>(e.yoffset);
  // }
  //
  // auto CameraSystem::GetState() -> State&
  // {
  //   return _state;
  // }
  //
  // auto CameraSystem::GetState() const -> State const&
  // {
  //   return _state;
  // }
  //
  // auto CameraSystem::Invoke() -> void
  // {
  //   entt::registry reg;
  //
  //   auto camera = GetActiveCamera();
  //
  //   auto& camera_cmp = reg.get<Component::Camera>(camera);
  //   auto& transform  = reg.get<Component::Transform>(camera);
  //
  //   // Scroll Delta (zoom / speed)
  //   if (_state.scroll_delta != 0.0f)
  //   {
  //     if (camera_cmp.mode == Component::Camera::CameraMode::Orbital)
  //     {
  //       _state.orbital_distance -= _state.scroll_delta * _state.zoom_speed;
  //       _state.orbital_distance  = glm::max(_state.orbital_distance, 0.5f);
  //     }
  //     else if (camera_cmp.mode == Component::Camera::CameraMode::Fly)
  //     {
  //       // Nothing for now
  //     }
  //     _state.scroll_delta = 0.0f; // consume
  //   }
  //
  //   if (camera_cmp.mode == Component::Camera::CameraMode::Orbital)
  //   {
  //
  //   }
  // }
  //
  //
  // auto CameraSystem::FlyCameraController::OnMouseMove(Event::MouseMove const& e, Component::Transform& transform, State& state)     noexcept -> void
  // {
  //    if (not state.left_button_down) return;
  //
  //    if (state.first_mouse)
  //    {
  //      state.last_x      = e.xpos;
  //      state.last_y      = e.ypos;
  //      state.first_mouse = false;
  //      return;
  //    }
  //
  //    float dx = static_cast<float>(e.xpos - state.last_x);
  //    float dy = static_cast<float>(e.ypos - state.last_y);
  //
  //    dx *= state.rotation_sensitivity;
  //    dy *= state.rotation_sensitivity;
  //
  //    glm::quat yaw_delta   = glm::angleAxis(dx, glm::vec3(0.0f, 1.0f, 0.0f));
  //    glm::vec3 right       = transform.rotation * glm::vec3(1.0f, 0.0f, 0.0f);
  //    glm::quat pitch_delta = glm::angleAxis(dy, right);
  //
  //    transform.rotation = yaw_delta * transform.rotation * pitch_delta;
  //    transform.rotation = glm::normalize(transform.rotation);
  //
  //    state.last_x      = e.xpos;
  //    state.last_y      = e.ypos;
  //    state.first_mouse = false;
  // }
  //
  // auto CameraSystem::FlyCameraController::OnMouseButton(Event::MouseButton const& e, State& state) noexcept -> void
  // {
  //   if (e.button == GLFW_MOUSE_BUTTON_LEFT)
  //   {
  //     state.left_button_down = (e.action == GLFW_PRESS);
  //
  //     if (state.left_button_down)
  //     {
  //       state.first_mouse = true;
  //     }
  //   }
  // }
  //
  // auto CameraSystem::OrbitalCameraController::OnMouseMove(Event::MouseMove const& e, Component::Transform& transform, State& state)     noexcept -> void
  // {
  //   if (not state.left_button_down) return;
  //
  //   if (state.first_mouse)
  //   {
  //     state.last_x      = e.xpos;
  //     state.last_y      = e.ypos;
  //     state.first_mouse = false;
  //     return;
  //   }
  // }
  //
  // auto CameraSystem::OrbitalCameraController::OnMouseButton(Event::MouseButton const& e, Component::Transform& transform, State& state) noexcept -> void
  // {
  //
  // }

  CameraSystem::CameraSystem()
  {
    _on_resize = Application::GetInstance().GetEventBus().sink<Event::Resize>().connect<&CameraSystem::OnResize>(this);
  }

  auto CameraSystem::OnResize(Event::Resize const& e) -> void
  {
    auto& scene = ServiceLocator::GetInstance().Get<SceneManagerService>().GetActiveScene();
    auto& reg   = scene.registry;
    auto cam    = scene.GetActiveCamera();

    if (cam == entt::null)
    {
      return;
    }

    auto& proj = reg.get<Component::Projection>(cam);

    if (auto* pers = std::get_if<Component::PerspectiveData>(&proj.data))
    {
      pers->aspect = static_cast<float>(e.width) / static_cast<float>(e.height);
    }
    else if (auto* ortho = std::get_if<Component::OrthographicData>(&proj.data))
    {
      float vertical_size = ortho->top - ortho->bottom;
      float aspect = static_cast<float>(e.width) / static_cast<float>(e.height);
      float horizontal_size = vertical_size * aspect;
      ortho->left = -horizontal_size / 2.0f;
      ortho->right =  horizontal_size / 2.0f;
    }
  }

  auto CameraSystem::Invoke() -> void
  {
    // NOTE: Maybe later we can update the matrix cache
  }

}
