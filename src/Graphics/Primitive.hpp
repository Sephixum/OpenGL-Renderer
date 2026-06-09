#pragma once

#include "Model.hpp"

namespace glr::Primitive
{
  [[nodiscard]] auto Cube()                                     -> ModelData;
  [[nodiscard]] auto Cylinder(u32 segments = 32u)               -> ModelData;
  [[nodiscard]] auto Sphere(u32 rings = 16u, u32 sectors = 32u) -> ModelData;
}
