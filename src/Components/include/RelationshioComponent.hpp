#pragma once

#include <entt/entity/entity.hpp>

namespace glr::Component
{

  struct RelationShip
  {
    entt::entity parent = entt::null;
  };

}
