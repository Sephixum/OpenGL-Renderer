#pragma once

#include <glm/glm.hpp>

namespace glr
{
  struct VertexData
  {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
  };

  // std430 pad rules check
  static_assert(
      sizeof(VertexData) ==   (sizeof(float) * 3) // position
                            + (sizeof(float) * 3) // normal
                            + (sizeof(float) * 2) // uv
  );

}
