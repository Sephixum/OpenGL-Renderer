#include "Utils/Error.hpp"
#include "Core/Application.hpp"

#include "FlyCameraControllerSystem.hpp"

#include "Components/Components.hpp"
#include "Events/Event.hpp"

#include "Services/ServiceLocator.hpp"
#include "Services/InputManagerService.hpp"
#include "Services/SceneManagerService.hpp"
#include "Services/TimerService.hpp"

#include <GLFW/glfw3.h>
#include <entt/entt.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>

namespace glr
{

  FlyCameraControllerSystem::FlyCameraControllerSystem()
  {
    _camera_switch_sink = Application::GetInstance().GetEventBus().sink<Event::CameraSwitch>().connect<&FlyCameraControllerSystem::CameraSwitched>(this);
  }

  auto FlyCameraControllerSystem::Invoke() -> void
  {
    auto& scene = ServiceLocator::GetInstance().Get<SceneManagerService>().GetActiveScene();
    auto& reg   = scene.registry;
    auto cam    = scene.GetActiveCamera();

    auto& input_service = ServiceLocator::GetInstance().Get<InputManagerService>();
    auto& timer_service = ServiceLocator::GetInstance().Get<TimerService>();

    if (cam == entt::null)
    {
      return;
    }

    auto& transform = reg.get<Component::Transform>(cam);
    auto  dt        = timer_service.GetDeltaSeconds();

    auto mouse_delta = input_service.GetMouseDelta();

    if (not _state.angles_initialised)
    {
      glm::vec3 euler = glm::eulerAngles(transform.rotation);
      _state.yaw                = euler.y;   // yaw (around Y)
      _state.pitch              = euler.x;   // pitch (around X)
      _state.angles_initialised = true;
    }

    if (input_service.IsMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT))
    {
      if (mouse_delta.x != 0.0f || mouse_delta.y != 0.0f)
      {
        _state.yaw   -= mouse_delta.x * _state.look_sensitivity * dt;   // left/right
        _state.pitch -= mouse_delta.y * _state.look_sensitivity * dt;   // up/down
        _state.pitch  = glm::clamp(_state.pitch,
                                   glm::radians(-89.0f),
                                   glm::radians(89.0f));
      }
    }

    // This order (yaw then pitch) guarantees no roll and keeps the camera upright.
    glm::quat q_yaw    = glm::angleAxis(_state.yaw,   glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat q_pitch  = glm::angleAxis(_state.pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    transform.rotation = q_yaw * q_pitch;

    glm::vec3 forward = transform.rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 right   = transform.rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    forward.y         = 0.0f;
    forward           = glm::normalize(forward);

    auto move = glm::vec3(0.0f);
    if (input_service.IsKeyBoardKeyDown(GLFW_KEY_W))            move += forward;
    if (input_service.IsKeyBoardKeyDown(GLFW_KEY_S))            move -= forward;
    if (input_service.IsKeyBoardKeyDown(GLFW_KEY_D))            move += right;
    if (input_service.IsKeyBoardKeyDown(GLFW_KEY_A))            move -= right;
    if (input_service.IsKeyBoardKeyDown(GLFW_KEY_SPACE))        move += glm::vec3(0.0f, 1.0f, 0.0f);
    if (input_service.IsKeyBoardKeyDown(GLFW_KEY_LEFT_CONTROL)) move -= glm::vec3(0.0f, 1.0f, 0.0f);

    if (glm::length(move) > 0.0f)
    {
      move = glm::normalize(move) * _state.speed * static_cast<float>(dt);
      transform.position += move;
    }

     // 4. Mouse wheel to adjust speed
    float scroll = input_service.GetScrollDelta();
    if (scroll != 0.0f)
    {
      _state.speed += scroll * _state.speed_scroll_mult;
      _state.speed  = glm::max(_state.speed, 1.0f);
    }
  }

  auto FlyCameraControllerSystem::CameraSwitched(Event::CameraSwitch) -> void
  {
    _state.angles_initialised = false;
  }

}
