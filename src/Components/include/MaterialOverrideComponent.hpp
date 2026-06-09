#pragma once

#include <string>
#include <optional>

namespace glr::Component
{

  struct MaterialOverride
  {
    std::optional<std::string> albedo;
    std::optional<std::string> normal;
    std::optional<std::string> roughness;
    std::optional<std::string> metallic;
  };

}
