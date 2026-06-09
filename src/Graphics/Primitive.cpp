#pragma once

#include "Primitive.hpp"
#include "Model.hpp"

namespace
{
  [[nodiscard]] auto MakeSingleMeshModel(glr::MeshData mesh) -> glr::ModelData
  {
    auto model = glr::ModelData{};

    model.bounds = mesh.bounds;

    model.meshes.push_back(std::move(mesh));

    model.root.name = "Root";
    model.root.mesh_indices.push_back(0);

    return model;
  }
}

auto glr::Primitive::Sphere(u32 rings, u32 sectors) -> ModelData
{
  auto mesh = MeshData{};

  constexpr f32 radius = 0.5f;

  for (u32 r = 0; r <= rings; ++r)
  {
    f32 v = static_cast<f32>(r) / rings;
    f32 phi = v * std::numbers::pi_v<f32>;

    for (u32 s = 0; s <= sectors; ++s)
    {
      f32 u = static_cast<f32>(s) / sectors;
      f32 theta = u * std::numbers::pi_v<f32> * 2.0f;

      glm::vec3 pos =
      {
        radius * std::sin(phi) * std::cos(theta),
        radius * std::cos(phi),
        radius * std::sin(phi) * std::sin(theta)
      };

      mesh.vertices.push_back(
      {
        pos,
        glm::normalize(pos),
        {u, v}
      });
    }
  }

  for (u32 r = 0; r < rings; ++r)
  {
    for (u32 s = 0; s < sectors; ++s)
    {
      u32 cur  = r * (sectors + 1) + s;
      u32 next = cur + sectors + 1;

      mesh.indices.push_back(cur);
      mesh.indices.push_back(next);
      mesh.indices.push_back(next + 1);

      mesh.indices.push_back(cur);
      mesh.indices.push_back(next + 1);
      mesh.indices.push_back(cur + 1);
    }
  }

  mesh.bounds.min = {-radius,-radius,-radius};
  mesh.bounds.max = { radius, radius, radius};

  return MakeSingleMeshModel(std::move(mesh));
}

auto glr::Primitive::Cube() -> ModelData
{
  auto mesh = MeshData{};

  mesh.vertices =
  {
    // Front
    {{-0.5f,-0.5f, 0.5f}, { 0, 0, 1}, {0,0}},
    {{ 0.5f,-0.5f, 0.5f}, { 0, 0, 1}, {1,0}},
    {{ 0.5f, 0.5f, 0.5f}, { 0, 0, 1}, {1,1}},
    {{-0.5f, 0.5f, 0.5f}, { 0, 0, 1}, {0,1}},

    // Back
    {{ 0.5f,-0.5f,-0.5f}, { 0, 0,-1}, {0,0}},
    {{-0.5f,-0.5f,-0.5f}, { 0, 0,-1}, {1,0}},
    {{-0.5f, 0.5f,-0.5f}, { 0, 0,-1}, {1,1}},
    {{ 0.5f, 0.5f,-0.5f}, { 0, 0,-1}, {0,1}},

    // Left
    {{-0.5f,-0.5f,-0.5f}, {-1, 0, 0}, {0,0}},
    {{-0.5f,-0.5f, 0.5f}, {-1, 0, 0}, {1,0}},
    {{-0.5f, 0.5f, 0.5f}, {-1, 0, 0}, {1,1}},
    {{-0.5f, 0.5f,-0.5f}, {-1, 0, 0}, {0,1}},

    // Right
    {{ 0.5f,-0.5f, 0.5f}, { 1, 0, 0}, {0,0}},
    {{ 0.5f,-0.5f,-0.5f}, { 1, 0, 0}, {1,0}},
    {{ 0.5f, 0.5f,-0.5f}, { 1, 0, 0}, {1,1}},
    {{ 0.5f, 0.5f, 0.5f}, { 1, 0, 0}, {0,1}},

    // Top
    {{-0.5f, 0.5f, 0.5f}, { 0, 1, 0}, {0,0}},
    {{ 0.5f, 0.5f, 0.5f}, { 0, 1, 0}, {1,0}},
    {{ 0.5f, 0.5f,-0.5f}, { 0, 1, 0}, {1,1}},
    {{-0.5f, 0.5f,-0.5f}, { 0, 1, 0}, {0,1}},

    // Bottom
    {{-0.5f,-0.5f,-0.5f}, { 0,-1, 0}, {0,0}},
    {{ 0.5f,-0.5f,-0.5f}, { 0,-1, 0}, {1,0}},
    {{ 0.5f,-0.5f, 0.5f}, { 0,-1, 0}, {1,1}},
    {{-0.5f,-0.5f, 0.5f}, { 0,-1, 0}, {0,1}},
  };

  mesh.indices =
  {
     0, 1, 2,  2, 3, 0,
     4, 5, 6,  6, 7, 4,
     8, 9,10, 10,11, 8,
    12,13,14, 14,15,12,
    16,17,18, 18,19,16,
    20,21,22, 22,23,20
  };

  mesh.bounds.min = {-0.5f,-0.5f,-0.5f};
  mesh.bounds.max = { 0.5f, 0.5f, 0.5f};

  return MakeSingleMeshModel(std::move(mesh));
}

auto glr::Primitive::Cylinder(u32 segments) -> ModelData
{
  auto mesh = MeshData{};

  constexpr f32 radius      = 0.5f;
  constexpr f32 half_height = 0.5f;

  //
  // Side vertices
  //
  for (u32 i = 0; i <= segments; ++i)
  {
    f32 t = static_cast<f32>(i) / static_cast<f32>(segments);
    f32 angle = t * std::numbers::pi_v<f32> * 2.0f;

    f32 x = std::cos(angle) * radius;
    f32 z = std::sin(angle) * radius;

    glm::vec3 normal = glm::normalize(glm::vec3{x,0.0f,z});

    mesh.vertices.push_back({
      {x,-half_height,z},
      normal,
      {t,0.0f}
    });

    mesh.vertices.push_back({
      {x,half_height,z},
      normal,
      {t,1.0f}
    });
  }

  //
  // Side indices
  //
  for (u32 i = 0; i < segments; ++i)
  {
    u32 b0 = i * 2;
    u32 t0 = b0 + 1;
    u32 b1 = b0 + 2;
    u32 t1 = b0 + 3;

    mesh.indices.insert(mesh.indices.end(),
    {
      b0, t0, t1,
      b0, t1, b1
    });
  }

  //
  // Top cap
  //
  u32 top_center = static_cast<u32>(mesh.vertices.size());

  mesh.vertices.push_back({
    {0.0f,half_height,0.0f},
    {0.0f,1.0f,0.0f},
    {0.5f,0.5f}
  });

  for (u32 i = 0; i <= segments; ++i)
  {
    f32 t = static_cast<f32>(i) / segments;
    f32 angle = t * std::numbers::pi_v<f32> * 2.0f;

    f32 x = std::cos(angle) * radius;
    f32 z = std::sin(angle) * radius;

    mesh.vertices.push_back({
      {x,half_height,z},
      {0.0f,1.0f,0.0f},
      {(x / radius + 1.0f) * 0.5f,
       (z / radius + 1.0f) * 0.5f}
    });
  }

  for (u32 i = 0; i < segments; ++i)
  {
    mesh.indices.push_back(top_center);
    mesh.indices.push_back(top_center + i + 1);
    mesh.indices.push_back(top_center + i + 2);
  }

  //
  // Bottom cap
  //
  u32 bottom_center = static_cast<u32>(mesh.vertices.size());

  mesh.vertices.push_back({
    {0.0f,-half_height,0.0f},
    {0.0f,-1.0f,0.0f},
    {0.5f,0.5f}
  });

  for (u32 i = 0; i <= segments; ++i)
  {
    f32 t = static_cast<f32>(i) / segments;
    f32 angle = t * std::numbers::pi_v<f32> * 2.0f;

    f32 x = std::cos(angle) * radius;
    f32 z = std::sin(angle) * radius;

    mesh.vertices.push_back({
      {x,-half_height,z},
      {0.0f,-1.0f,0.0f},
      {(x / radius + 1.0f) * 0.5f,
       (z / radius + 1.0f) * 0.5f}
    });
  }

  for (u32 i = 0; i < segments; ++i)
  {
    mesh.indices.push_back(bottom_center);
    mesh.indices.push_back(bottom_center + i + 2);
    mesh.indices.push_back(bottom_center + i + 1);
  }

  mesh.bounds.min = {-radius,-half_height,-radius};
  mesh.bounds.max = { radius, half_height, radius};

  return MakeSingleMeshModel(std::move(mesh));
}
