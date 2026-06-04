
#include "Components/Components.hpp"
#include "Components/include/CameraComponent.hpp"
#include "Components/include/MeshAssetComponent.hpp"
#include "Components/include/ProjectionComponent.hpp"
#include "ISystem.hpp"
#include "RenderingSystem.hpp"
#include "Graphics/VertexArray.hpp"

#include "Graphics/GraphicsPipeline.hpp"
#include "Graphics/Buffer.hpp"

#include "Services/MeshManagerService.hpp"
#include "Services/ServiceLocator.hpp"
#include "Services/ShaderManagerService.hpp"
#include "Services/SceneManagerService.hpp"

#include <cstdint>
#include <functional>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/matrix.hpp>
#include <iterator>
#include <span>
#include <unordered_map>
#include <entt/entt.hpp>

namespace glr
{

  RenderingSystem::RenderingSystem()
    : _vao{"Dummy VertexArray"}
    , _gbuffer_pipeline("gbuffer GraphicsPipeline")
    , _indirect_buffer(10, "Indirect Command Buffer")
    , _instance_buffer(10, "Instance Data Buffer")
    , _camera_buffer(sizeof(CameraData), "Camera Data Buffer")
  {
    glEnable(GL_DEPTH_TEST);

    auto& shader_manager = ServiceLocator::GetInstance().Get<ShaderManagerService>();

    auto& gbuffer_vert_shader = shader_manager.GetVertexShader("gbuffer");
    auto& gbuffer_frag_shader = shader_manager.GetFragmentShader("gbuffer");
    _gbuffer_pipeline
      .SetVertexShader(gbuffer_vert_shader)
      .SetFragmentShader(gbuffer_frag_shader)
      .Activate();

  }

  auto RenderingSystem::Invoke() -> void
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (_state.gbuffer_pass) RenderGBuffer();
  }

  auto RenderingSystem::UpdateCameraBuffer() -> void
  {
    auto& scene = ServiceLocator::GetInstance().Get<SceneManagerService>().GetActiveScene();
    auto& reg   = scene.registry;
    auto cam    = scene.GetActiveCamera();

    if (cam == entt::null)
    {
      return;
    }

    auto [projection, transform] = reg.get<Component::Projection, Component::Transform>(cam);

    auto view_mat = glm::inverse(transform.GetMatrix());
    auto proj_mat = projection.GetMatrix();

    auto camera_data = CameraData{view_mat, proj_mat};
    _camera_buffer.WriteData(std::as_bytes(std::span{&camera_data, 1}));
  }

  auto RenderingSystem::RenderGBuffer() -> void
  {
    static constexpr auto range = std::views::iota;

    auto& mesh_manager = ServiceLocator::GetInstance().Get<MeshManagerService>();
    auto& reg          = ServiceLocator::GetInstance().Get<SceneManagerService>().GetActiveScene().registry;

    auto view = reg.view<Component::MeshAsset, Component::Transform>();

    if (std::ranges::distance(view.each()) == 0)
    {
      return;
    }

    struct SubMeshKey 
    {
      std::string_view tag = {};
      std::size_t sub_idx  = {};
      auto operator<=>(SubMeshKey const&) const = default;
    };

    auto sub_mesh_key_hash_fn = [](SubMeshKey const& k) static -> std::size_t
    {
      std::size_t h1 = std::hash<std::string_view>{}(k.tag);
      std::size_t h2 = std::hash<std::size_t>{}(k.sub_idx);
      return h1 ^ (h2 << 1);
    };

    auto grouped = std::unordered_map<SubMeshKey, std::vector<InstanceData>, decltype(sub_mesh_key_hash_fn)>{};

    for (auto [entity, mesh, transform] : view.each())
    {
      if (not mesh_manager.IsMeshLoaded(mesh.tag))
      {
        continue;
      }

      auto const mesh_views   = mesh_manager.GetMeshData(mesh.tag);
      auto const model_matrix = transform.GetMatrix();

      for (auto i : range(0zu, mesh_views.size()))
      {
        grouped[{mesh.tag, i}].push_back({model_matrix});
      }

    }

    if (grouped.empty())
    {
      return;
    }

    _indirect_buffer.Clear();
    _instance_buffer.Clear();

    for (auto const& [key, model_matrices] : grouped)
    {
      auto const  mesh_views = mesh_manager.GetMeshData(key.tag);
      auto const& submesh    = mesh_views[key.sub_idx];

      auto const base_instance = static_cast<std::uint32_t>(_instance_buffer.Size());
      _instance_buffer.Append(model_matrices);

      auto cmd = DrawIndirectCommand
      {
        .count          = submesh.index_count,
        .instance_count = static_cast<std::uint32_t>(model_matrices.size()),
        .first_index    = submesh.index_offset,
        .base_vertex    = static_cast<std::int32_t>(submesh.vertex_offset),
        .base_instance  = base_instance
      };

      _indirect_buffer.Append(cmd);
    }

    UpdateCameraBuffer();

    _vao.BuildSettings()
      .BindAs<BufferType::ShaderStorage>(mesh_manager.GetVertexBuffer(), 0)
      .BindAs<BufferType::ShaderStorage>(_camera_buffer, 1)
      .BindAs<BufferType::ShaderStorage>(_instance_buffer, 2)
      .BindAs<BufferType::Index>(mesh_manager.GetIndexBuffer())
      .BindAs<BufferType::DrawIndirect>(_indirect_buffer)
      .Apply()
      .Activate();

    _gbuffer_pipeline.Activate();

    ::glMultiDrawElementsIndirect(
        GL_LINES,
        GL_UNSIGNED_INT,
        nullptr,
        _indirect_buffer.Size(),
        0
    );
  }

}

