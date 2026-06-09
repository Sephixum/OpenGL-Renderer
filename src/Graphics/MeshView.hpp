#pragma once

#include "Utils/Formatter.hpp"
#include <glm/glm.hpp>
#include <cstdint>
#include <format>

namespace glr
{

  struct MeshView
  {
    std::uint32_t index_offset;
    std::uint32_t index_count;
    std::uint32_t vertex_offset;
    std::uint32_t vertex_count;
    std::uint32_t material_index;
    glm::mat4     node_transform = glm::mat4(1.0f);
  };

  [[nodiscard]] constexpr auto to_string(MeshView const& v)
  {
    return std::format("({}, {}, {}, {})", v.index_offset, v.index_count, v.vertex_offset, v.vertex_count, v.material_index);
  }
  static_assert(CanFormat<MeshView>);

}
