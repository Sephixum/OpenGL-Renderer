#pragma once

#include "Utils/Overload.hpp"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/ext.hpp>
#include <variant>

namespace glr::Component
{

  struct PerspectiveData
  {
    float fov_y      = glm::radians(90.0f);
    float aspect     = 16.0f / 9.0f;
    float near_plane = 0.1f;
    float far_plane  = 100.0f;
  };
  
  struct OrthographicData
  {
    float left   = -10.0f;
    float right  = 10.0f;
    float bottom = -10.0f;
    float top    = 10.0f;
    float near_z = -1.0f;
    float far_z  = 1.0f;
  };

  struct Projection 
  {
    std::variant<PerspectiveData, OrthographicData> data = PerspectiveData{};  
                                                 
    auto GetMatrix() const -> glm::mat4
    {
      return std::visit(
        Overload
        {
          [](PerspectiveData const& pers) -> glm::mat4
          {
            return glm::perspective(pers.fov_y, pers.aspect, pers.near_plane, pers.far_plane);
          },
          [](OrthographicData const& ortho) -> glm::mat4
          {
            return glm::ortho(ortho.left, ortho.right, ortho.bottom, ortho.top, ortho.near_z, ortho.far_z);
          }
        }, data);
    }

    auto SetProjection(PerspectiveData const& perspective) -> void
    {
        data = perspective;
    }

    auto SetProjection(OrthographicData const& ortho) -> void
    {
        data = ortho;
    }

  };

}
