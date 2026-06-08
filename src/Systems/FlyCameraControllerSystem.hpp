#pragma once

#include "Core/Event.hpp"
#include "Core/EventBus.hpp"
#include "ISystem.hpp"
#include "Components/Components.hpp"
#include <entt/entity/fwd.hpp>
#include <entt/signal/sigh.hpp>

namespace glr
{

  class FlyCameraControllerSystem : public ISystem
  {
    struct State 
    {
      // Movement
      float speed             = 5.0f;
      float speed_scroll_mult = 0.5f;

      // Sensitivity
      float look_sensitivity = 0.2f;

      // Persistent angles (radians)
      float yaw   = 0.0f;
      float pitch = 0.0f;

      // (Optional) to initialise once from the scene’s camera rotation
      bool angles_initialised = false;
    }
    _state = {};

    EventSink _camera_switch_sink = {};

    auto CameraSwitched(Event::CameraSwitch) -> void;

    public:
      FlyCameraControllerSystem();
      virtual auto Invoke() -> void override;
  };

}
