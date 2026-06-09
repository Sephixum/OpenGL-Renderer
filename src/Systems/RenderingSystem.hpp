#pragma once

#include "Core/EventBus.hpp"
#include "Graphics/FrameBuffer.hpp"
#include "Graphics/VertexArray.hpp"
#include "ISystem.hpp"
#include "Graphics/GraphicsPipeline.hpp"
#include "Graphics/Buffer.hpp"
#include "Utils/Utils.hpp"

#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_uint2.hpp>

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

  struct alignas(16) GPUMaterial 
  {
    ::GLuint64 albedo_handle    = {};
    ::GLuint64 normal_handle    = {};
    ::GLuint64 roughness_handle = {};
    ::GLuint64 metallic_handle  = {};
  };

  struct alignas(16) InstanceData
  {
    glm::mat4 model = glm::mat4(1.0f);
  };

  struct alignas(16) CameraData
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

    EventSink _material_buffer_rebuild_sink = {};

    struct 
    {
      VertexArray                                  vao;
      GraphicsPipeline                             pipeline;
      FrameBuffer                                  frame_buffer;
      DynamicPersistantBuffer<DrawIndirectCommand> indirect_buffer;
      DynamicPersistantBuffer<InstanceData>        instance_buffer;
      DynamicPersistantBuffer<CameraData>          camera_buffer;
      DynamicPersistantBuffer<GPUMaterial>         material_buffer;
      DynamicPersistantBuffer<u32>                 draw_material_indices_buffer;
    }
    _geometry;

    struct
    {
      VertexArray                         vao;
      GraphicsPipeline                    pipeline;
      FrameBuffer                         frame_buffer;
      DynamicPersistantBuffer<::GLuint64> geometry_bindless_handle_buffer;
    }
    _lighting;


    auto RenderGBufferPass() -> void;
    auto RenderLightingPass() -> void;
    auto UpdateCameraBuffer() -> void;
    auto BuildGlobalMaterials() -> void;

    public:
      RenderingSystem();
      virtual auto Invoke() -> void override;

      [[nodiscard]] auto GetRenderState() -> State& { return _state; }
  };

}
