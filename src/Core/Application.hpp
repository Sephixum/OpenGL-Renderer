#pragma once

#include "Utils/Singleton.hpp"
#include "EventBus.hpp"

#include <entt/entt.hpp>
#include <entt/signal/fwd.hpp>

namespace glr
{

  class Application : public Singleton<Application>
  {
    EventBus _event_bus = {};

    auto Setup() -> void;
    auto CleanUp() -> void;

    public:
      auto Run() -> void;
      auto GetEventBus() -> EventBus&;
  };

}
