#pragma once

#include <optional>
#include <string>

namespace glr::Component
{

  struct MeshAsset
  {
    std::string                mesh_tag;
    std::optional<std::string> albedo_texture_tag    = {};
    std::optional<std::string> normal_texture_tag    = {};
    std::optional<std::string> roughness_texture_tag = {};
    std::optional<std::string> metalic_texture_tag   = {};
    std::optional<std::string> emmisive_texture_tag  = {};
  };

}
