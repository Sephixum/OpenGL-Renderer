#pragma once

#include <entt/signal/fwd.hpp>

namespace glr
{

  using EventBus  = entt::dispatcher;
  using EventSink = entt::scoped_connection;

}
