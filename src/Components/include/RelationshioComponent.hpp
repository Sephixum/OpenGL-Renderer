#pragma once

#include <entt/entity/entity.hpp>

namespace glr::Component
{

  struct RelationShip
  {
    entt::entity parent        = entt::null;
    entt::entity first_child   = entt::null;
    entt::entity next_sibgling = entt::null;
  };

}
