#pragma once

#include "Components/Components.hpp"
#include "Core/Event.hpp"
#include "Core/EventBus.hpp"
#include "ISystem.hpp"
#include "Core/Application.hpp"
#include "Components/Components.hpp"
#include <entt/entity/fwd.hpp>
#include <variant>

namespace glr
{

  // NOTE: Maybe moving all of these states to InputManagerService ?

  // class CameraSystem : public ISystem
  // {
  //   struct State 
  //   {
  //     // Mouse tracking
  //     double       last_x                 = 0.0;
  //     double       last_y                 = 0.0;
  //     bool         first_mouse            = true;
  //     bool         left_button_down       = false;
  //
  //     // Scroll
  //     float        scroll_delta           = 0.0f;
  //
  //     // Sensitivity settings
  //     float        rotation_sensitivity   = 0.005f;
  //     float        zoom_speed             = 0.5f;
  //
  //     // Orbital specific
  //     entt::entity target                 = entt::null;
  //     float        orbital_distance       = 10.0f;
  //     float        orbital_yaw            = 0.0f;
  //     float        orbital_pitch          = 30.0f;
  //
  //     // Fly related
  //     float fly_speed = 5.0f;
  //   }
  //   _state = {};
  //
  //   struct FlyCameraController
  //   {
  //     static auto OnMouseMove(Event::MouseMove const& e, Component::Transform& transform, State& state)     noexcept -> void;
  //     static auto OnMouseButton(Event::MouseButton const& e, State& state)                                  noexcept -> void;
  //   };
  //
  //   struct OrbitalCameraController
  //   {
  //     static auto OnMouseMove(Event::MouseMove const& e, Component::Transform& transform, State& state)     noexcept -> void;
  //     static auto OnMouseButton(Event::MouseButton const& e, Component::Transform& transform, State& state) noexcept -> void;
  //   };
  //
  //   EventSink _mouse_move_sink;
  //   EventSink _mouse_scroll_sink;
  //   EventSink _mouse_button_sink;
  //
  //   auto OnMouseMove(Event::MouseMove const& e)     noexcept -> void;
  //   auto OnMouseButton(Event::MouseButton const& e) noexcept -> void;
  //   auto OnMouseScroll(Event::MouseScroll const& e) noexcept -> void;
  //
  //   [[nodiscard]] auto GetActiveCamera() -> entt::entity;
  //
  //   public:
  //     CameraSystem();
  //     virtual auto Invoke() -> void override;
  //
  //     auto GetState()       -> State&;
  //     auto GetState() const -> State const&;
  // };


  class CameraSystem : public ISystem
  {
    EventSink _on_resize;

    auto OnResize(Event::Resize const& e) -> void;

    public:
      CameraSystem();
      virtual auto Invoke() -> void override;
  };

}
