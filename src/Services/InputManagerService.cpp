#include "InputManagerService.hpp"

#include "Core/Application.hpp"
#include "Events/Event.hpp"

#include "Services/ServiceLocator.hpp"
#include "Services/WindowService.hpp"

#include <GLFW/glfw3.h>


namespace glr
{

  InputManagerService::InputManagerService()
  {
    _mouse_move_sink   = Application::GetInstance().GetEventBus().sink<Event::MouseMove>().connect<& InputManagerService::OnMouseMove>(this);
    _mouse_button_sink = Application::GetInstance().GetEventBus().sink<Event::MouseButton>().connect<&InputManagerService::OnMouseButton>(this);
    _mouse_scroll_sink = Application::GetInstance().GetEventBus().sink<Event::MouseScroll>().connect<&InputManagerService::OnMouseScroll>(this);
    _keyboard_key_sink = Application::GetInstance().GetEventBus().sink<Event::KeyBoardKey>().connect<&InputManagerService::OnKeyboardKey>(this);
  }

  auto InputManagerService::OnMouseMove(Event::MouseMove const& e) -> void 
  {
    if (not _state.enabled) return;

    if (_state.first_mouse) 
    {
      _state.last_x      = e.xpos;
      _state.last_y      = e.ypos;
      _state.first_mouse = false;
      return;
    }

    double dx = e.xpos - _state.last_x;
    double dy = e.ypos - _state.last_y;

    _state.mouse_delta.x += static_cast<float>(dx);
    _state.mouse_delta.y += static_cast<float>(dy);

    _state.last_x = e.xpos;
    _state.last_y = e.ypos;
  }

  auto InputManagerService::OnMouseButton(Event::MouseButton const& e) -> void 
  {
    if (not _state.enabled) return;

    if ((e.button >= 0) and (e.button <= k_max_mouse_keys))
    {
      _state.mouse_keys.set(e.button, (e.action == GLFW_PRESS) or (e.action == GLFW_REPEAT));
    }
  }

  auto InputManagerService::OnMouseScroll(Event::MouseScroll const& e) -> void 
  {
    if (!_state.enabled) return;
    _state.scroll_delta += static_cast<float>(e.yoffset);
  }

  auto InputManagerService::OnKeyboardKey(Event::KeyBoardKey const& e) -> void 
  {
    if (!_state.enabled) return;

    if (e.key >= 0 && e.key <= k_max_keyboard_keys)
    {
      _state.keyboard_keys.set(e.key, (e.action == GLFW_PRESS) or (e.action == GLFW_REPEAT));
    }
  }

  auto InputManagerService::GetMouseDelta() -> glm::vec2
  {
    glm::vec2 d        = _state.mouse_delta;
    _state.mouse_delta = glm::vec2(0.0f);
    return d;
  }

  auto InputManagerService::GetScrollDelta() -> float
  {
    float val           = _state.scroll_delta;
    _state.scroll_delta = 0.0f;
    return val;
  }

  auto InputManagerService::IsMouseButtonDown(int button) const -> bool
  {
    if (button < 0 || button > k_max_mouse_keys) return false;
    return _state.mouse_keys.test(button);
  } 

  auto InputManagerService::IsKeyBoardKeyDown(int key) const -> bool
  {
    if (key < 0 || key > k_max_keyboard_keys) return false;
    return _state.keyboard_keys.test(key);
  }

  auto InputManagerService::SetEnabled(bool enabled) -> void
  {
    _state.enabled = enabled;
    if (not _state.enabled)
    {
      _state.mouse_delta = glm::vec2(0.0f);
      _state.scroll_delta = 0.0f;
      _state.first_mouse = true;
      _state.keyboard_keys.reset();
      _state.mouse_keys.reset();
    }
  }

  auto InputManagerService::IsEnabled() const -> bool
  {
    return _state.enabled;
  }

  auto InputManagerService::OnInit() -> void {}

  auto InputManagerService::OnUpdate() -> void {}

  auto InputManagerService::OnShutdown() -> void {}


}
