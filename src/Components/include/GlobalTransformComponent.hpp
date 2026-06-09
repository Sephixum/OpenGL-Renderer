#pragma once

#include <glm/glm.hpp>

namespace glr::Component
{

  struct GlobalTransform
  {
    glm::mat4 matrix = glm::mat4(1.0f);
  };

}
