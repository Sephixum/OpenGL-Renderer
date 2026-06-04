#pragma once

#include "Services/IService.hpp"
#include "Graphics/MeshData.hpp"

namespace glr
{

  class MeshFactoryService : public IService
  {
    public:
      virtual auto OnInit()     -> void override {}
      virtual auto OnUpdate()   -> void override {}
      virtual auto OnShutdown() -> void override {}

    [[nodiscard]] constexpr auto CreateCube() -> MeshData
    {
      std::vector<VertexData> vertices;
      std::vector<std::uint32_t> indices;

      auto add_face = [&](glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
      {
        // Compute face normal from the first triangle (p0,p1,p2)
        glm::vec3 normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));

        std::array<UV, 4> uvs = {{{0,0}, {1,0}, {1,1}, {0,1}}};

        auto start_index = static_cast<std::uint32_t>(vertices.size());

        vertices.push_back({p0, normal, uvs[0]});
        vertices.push_back({p1, normal, uvs[1]});
        vertices.push_back({p2, normal, uvs[2]});
        vertices.push_back({p3, normal, uvs[3]});

        // Two triangles: (0,1,2) and (0,2,3)
        indices.push_back(start_index + 0);
        indices.push_back(start_index + 1);
        indices.push_back(start_index + 2);

        indices.push_back(start_index + 0);
        indices.push_back(start_index + 2);
        indices.push_back(start_index + 3);
      };

      // Front face (+z)
      add_face({-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1});
      // Back face (-z) – winding order ensures outward normal
      add_face({ 1, -1, -1}, {-1, -1, -1}, {-1,  1, -1}, { 1,  1, -1});
      // Top face (+y)
      add_face({-1,  1,  1}, { 1,  1,  1}, { 1,  1, -1}, {-1,  1, -1});
      // Bottom face (-y)
      add_face({-1, -1, -1}, { 1, -1, -1}, { 1, -1,  1}, {-1, -1,  1});
      // Right face (+x)
      add_face({ 1, -1,  1}, { 1, -1, -1}, { 1,  1, -1}, { 1,  1,  1});
      // Left face (-x)
      add_face({-1, -1, -1}, {-1, -1,  1}, {-1,  1,  1}, {-1,  1, -1});

      return { vertices, indices };
    }

  };

}
