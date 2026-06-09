#pragma once

#include "Components/include/ProjectionComponent.hpp"
#include <utility>

namespace glr::Component
{

  // NOTE: these components are used as tags to retrieve from registry.

  struct Camera
  {
    enum struct CameraMode
    {
      Orbital,
      Fly
    } 
    mode                  = CameraMode::Fly;
    bool       primary    = false;
    Projection projection = {};
  };

  struct ActiveCamera
  {

  };

  [[nodiscard]] constexpr auto to_string(Camera::CameraMode m)
  {
    switch (m)
    {
      using enum Camera::CameraMode;
      case Orbital: return "Orbital";
      case Fly    : return "Fly";
    }
    std::unreachable();
  }

}
