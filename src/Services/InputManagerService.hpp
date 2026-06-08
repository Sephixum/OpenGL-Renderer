#pragma once

#include "IService.hpp"
#include "Events/Event.hpp"
#include "Events/EventBus.hpp"
#include <bitset>
#include <entt/signal/sigh.hpp>
#include <glm/ext/vector_float2.hpp>
#include <GLFW/glfw3.h>

namespace glr
{

  class InputManagerService : public IService
  {
    auto OnMouseMove(Event::MouseMove const& e)     -> void;
    auto OnMouseButton(Event::MouseButton const& e) -> void;
    auto OnMouseScroll(Event::MouseScroll const& e) -> void;
    auto OnKeyboardKey(Event::KeyBoardKey const& e) -> void;

    EventSink _mouse_move_sink;
    EventSink _mouse_button_sink;
    EventSink _mouse_scroll_sink;
    EventSink _keyboard_key_sink;

    static constexpr auto k_max_keyboard_keys = GLFW_KEY_LAST;
    static constexpr auto k_max_mouse_keys    = GLFW_MOUSE_BUTTON_LAST;

    struct State
    {
      double    last_x       = 0.0;
      double    last_y       = 0.0;
      bool      first_mouse  = true;
      glm::vec2 mouse_delta  = glm::vec2(0.0f);
      float     scroll_delta = 0.0f;

      std::bitset<k_max_keyboard_keys + 1> keyboard_keys = {};
      std::bitset<k_max_mouse_keys + 1>    mouse_keys    = {};

      bool mouse_enabled    = true;
      bool keyboard_enabled = true;
    }
    _state = {};

    public:
      InputManagerService();

      virtual auto OnInit()     -> void override;
      virtual auto OnUpdate()   -> void override;
      virtual auto OnShutdown() -> void override;

      [[nodiscard]] auto GetMouseDelta()                     -> glm::vec2;
      [[nodiscard]] auto GetScrollDelta()                    -> float;
      [[nodiscard]] auto IsMouseButtonDown(int button) const -> bool;
      [[nodiscard]] auto IsKeyBoardKeyDown(int key)    const -> bool;

      auto SetMouseEnabled(bool enabled)        -> void;
      auto SetKeyboardEnabled(bool enabled)        -> void;
      [[nodiscard]] auto IsMouseEnabled() const -> bool;
      [[nodiscard]] auto IsKeyboardEnabled() const -> bool;
  };

}
