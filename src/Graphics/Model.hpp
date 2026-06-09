#pragma once

#include "VertexData.hpp"
#include "AABB.hpp"
#include "Utils/Utils.hpp"
#include <glm/ext/matrix_float4x4.hpp>
#include <vector>

namespace glr
{

  struct MeshData
  {
    std::vector<VertexData> vertices;
    std::vector<u32>        indices;

    AABB                    bounds         = {};
    u32                     material_index = 0u;
  };

  struct NodeData
  {
    std::string name;

    glm::mat4             local_transform = glm::mat4(1.0f);
    std::vector<u32>      mesh_indices;
    std::vector<NodeData> children;
  };

  struct ModelData
  {
    std::vector<MeshData> meshes;
    NodeData              root;
    AABB                  bounds;
  };

  struct MeshView
  {
    u32 index_offset = 0u;
    u32 index_count  = 0u;

    u32 vertex_offset = 0u;
    u32 vertex_count  = 0u;

    u32 material_index = 0u;
  };

  struct ModelView
  {
    std::vector<MeshView> meshes;
    NodeData              root;
    AABB                  bounds;
  };


}
