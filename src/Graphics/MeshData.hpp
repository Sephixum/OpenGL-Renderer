#pragma once

#include "VertexData.hpp"
#include <vector>

namespace glr
{

  struct MeshData
  {
    std::vector<VertexData>    vertices;
    std::vector<std::uint32_t> indices;
  };

}
