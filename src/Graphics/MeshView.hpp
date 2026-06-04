#pragma once

#include "Utils/Formatter.hpp"
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
  };

  [[nodiscard]] constexpr auto to_string(MeshView const& v)
  {
    return std::format("({}, {}, {}, {})", v.index_offset, v.index_count, v.vertex_offset, v.vertex_count);
  }
  static_assert(CanFormat<MeshView>);

}
