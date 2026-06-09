#pragma once

#include "VertexData.hpp"
#include <vector>

namespace glr
{

  struct MeshData
  {
    std::vector<VertexData>    vertices;
    std::vector<std::uint32_t> indices;
    std::uint32_t              material_index;
    glm::mat4                  node_transform;
  };

}
