#pragma once

#include "Utils/Utils.hpp"
#include <glm/glm.hpp>
#include <limits>

namespace glr
{

  struct AABB
  {
    glm::vec3 min = glm::vec3(std::numeric_limits<f32>::max());
    glm::vec3 max = glm::vec3(std::numeric_limits<f32>::lowest());

    auto Expand(glm::vec3 const& p) -> void
    {
      min = glm::min(min, p);
      max = glm::max(max, p);
    }

    [[nodiscard]] static constexpr auto MergeAABBs(AABB const& a, AABB const& b) -> AABB
    {
        AABB result;
        result.min = glm::min(a.min, b.min);
        result.max = glm::max(a.max, b.max);
        return result;
    }
  };

}
