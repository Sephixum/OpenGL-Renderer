#pragma once

#include "Graphics/VertexArray.hpp"
#include "ISystem.hpp"
#include "Graphics/GraphicsPipeline.hpp"
#include "Graphics/Buffer.hpp"
#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>

namespace glr
{

  struct DrawIndirectCommand
  {
    std::uint32_t count;
    std::uint32_t instance_count;
    std::uint32_t first_index;
    std::int32_t  base_vertex;
    std::uint32_t base_instance;
  };

  struct InstanceData
  {
    glm::mat4 model = glm::mat4(1.0f);
  };

  struct CameraData
  {
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::mat4(1.0f);
  };

  class RenderingSystem : public ISystem
  {
    struct State
    {
      bool gbuffer_pass  = true;
      bool lighting_pass = true;
    }
    _state = {};

    VertexArray      _vao;
    GraphicsPipeline _gbuffer_pipeline;

    DynamicPersistantBuffer<DrawIndirectCommand> _indirect_buffer;
    DynamicPersistantBuffer<InstanceData>        _instance_buffer;
    DynamicPersistantBuffer<CameraData>          _camera_buffer;

    auto RenderGBuffer() -> void;
    auto UpdateCameraBuffer() -> void;

    public:
      RenderingSystem();
      virtual auto Invoke() -> void override;

      [[nodiscard]] auto GetRenderState() -> State& { return _state; }
  };

}
