#include "Components/Components.hpp"

#include "RenderingSystem.hpp"

#include "Core/Application.hpp"
#include "Core/Event.hpp"
#include "Graphics/VertexArray.hpp"
#include "Graphics/GraphicsPipeline.hpp"
#include "Graphics/Buffer.hpp"

#include "Services/MeshManagerService.hpp"
#include "Services/ServiceLocator.hpp"
#include "Services/ShaderManagerService.hpp"
#include "Services/SceneManagerService.hpp"
#include "Services/ResourceLoaderService.hpp"
#include "Services/TextureManagerService.hpp"

#include "Utils/InRangeOf.hpp"

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
    , _gbuffer_pipeline("Gbuffer GraphicsPipeline")
    , _indirect_buffer(10, "Indirect Command Buffer")
    , _instance_buffer(10, "Instance Data Buffer")
    , _camera_buffer(1, "Camera Data Buffer")
    , _material_buffer(10, "GlobalMaterials Buffer")
    , _draw_material_indices_buffer(256, "DrawMaterialIndices")
  {
    glEnable(GL_DEPTH_TEST);

    auto& shader_manager = ServiceLocator::GetInstance().Get<ShaderManagerService>();
    auto& mesh_manager   = ServiceLocator::GetInstance().Get<MeshManagerService>();

    auto& gbuffer_vert_shader = shader_manager.GetVertexShader("gbuffer");
    auto& gbuffer_frag_shader = shader_manager.GetFragmentShader("gbuffer");
    _gbuffer_pipeline
      .SetVertexShader(gbuffer_vert_shader)
      .SetFragmentShader(gbuffer_frag_shader)
      .Activate();


    _vao.BuildSettings()
      .BindAs<BufferType::ShaderStorage>(mesh_manager.GetVertexBuffer(), 0)
      .BindAs<BufferType::ShaderStorage>(_camera_buffer, 1)
      .BindAs<BufferType::ShaderStorage>(_instance_buffer, 2)
      .BindAs<BufferType::ShaderStorage>(_draw_material_indices_buffer, 3)
      .BindAs<BufferType::ShaderStorage>(_material_buffer, 4)
      .BindAs<BufferType::Index>(mesh_manager.GetIndexBuffer())
      .BindAs<BufferType::DrawIndirect>(_indirect_buffer)
      .Apply()
      .Activate();

    BuildGlobalMaterials();
    _material_buffer_rebuild_sink = Application::GetInstance().GetEventBus().sink<Event::MaterialLoaded>().connect<&RenderingSystem::BuildGlobalMaterials>(this);
  }

  auto RenderingSystem::Invoke() -> void
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (_state.gbuffer_pass) RenderGBuffer();
  }

  auto RenderingSystem::BuildGlobalMaterials() -> void
  {
    auto& res_ldr = ServiceLocator::GetInstance().Get<ResourceLoaderService>();
    auto& tex_mgr = ServiceLocator::GetInstance().Get<TextureManagerService>();
    _material_buffer.Clear();

    for (auto const& mat : res_ldr.GetGlobalMaterial())
    {
      auto gpu = GPUMaterial{};
      if (auto* tex = (not mat.albedo_tag.empty() ? tex_mgr.TeyGetTexture2D(mat.albedo_tag) : nullptr))
      {
        gpu.albedo_handle = tex->GetBindlessHandle();
      }
      if (auto* tex = (not mat.normal_tag.empty() ? tex_mgr.TeyGetTexture2D(mat.normal_tag) : nullptr))
      {
        gpu.normal_handle = tex->GetBindlessHandle();
      }
      if (auto* tex = (not mat.roughness_tag.empty() ? tex_mgr.TeyGetTexture2D(mat.roughness_tag) : nullptr))
      {
        gpu.roughness_handle = tex->GetBindlessHandle();
      }
      if (auto* tex = (not mat.metalic_tag.empty() ? tex_mgr.TeyGetTexture2D(mat.metalic_tag) : nullptr))
      {
        gpu.metallic_handle = tex->GetBindlessHandle();
      }
      _material_buffer.Append(gpu);
    }

    _vao.BuildSettings()
      .BindAs<BufferType::ShaderStorage>(_material_buffer, 4)
      .Apply();
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

    auto view_mat = transform.GetView();
    auto proj_mat = projection.GetMatrix();

    auto camera_data = CameraData{view_mat, proj_mat};
    _camera_buffer.WriteData(std::as_bytes(std::span{&camera_data, 1}));
  }

  auto RenderingSystem::RenderGBuffer() -> void
  {
    [[maybe_unused]] auto& res_ldr         = ServiceLocator::GetInstance().Get<ResourceLoaderService>();
    [[maybe_unused]] auto& mesh_manager    = ServiceLocator::GetInstance().Get<MeshManagerService>();
    [[maybe_unused]] auto& texture_manager = ServiceLocator::GetInstance().Get<TextureManagerService>();
    auto& reg             = ServiceLocator::GetInstance().Get<SceneManagerService>().GetActiveScene().registry;

    auto view = reg.view<Component::MeshAsset, Component::Transform>();

    if (std::ranges::distance(view.each()) == 0)
    {
      return;
    }

    struct SubMeshKey 
    {
      std::string tag = {};
      std::size_t sub_idx  = {};
      auto operator<=>(SubMeshKey const&) const = default;
    };

    auto sub_mesh_key_hash_fn = [](SubMeshKey const& k) static -> std::size_t
    {
      std::size_t h1 = std::hash<std::string>{}(k.tag);
      std::size_t h2 = std::hash<std::size_t>{}(k.sub_idx);
      return h1 ^ (h2 << 1);
    };

    auto grouped = std::unordered_map<SubMeshKey, std::vector<InstanceData>, decltype(sub_mesh_key_hash_fn)>{};

    for (auto [entity, mesh, transform] : view.each())
    {
      if (not mesh_manager.IsModelLoaded(mesh.mesh_tag))
      {
        continue;
      }

      auto const mesh_views   = mesh_manager.GetMeshData(mesh.mesh_tag);
      auto const model_matrix = transform.GetMatrix();

      for (auto i : InRangeOf(0zu, mesh_views.size()))
      {
        grouped[{mesh.mesh_tag, i}].push_back({model_matrix});
      }

    }

    if (grouped.empty())
    {
      return;
    }

    _indirect_buffer.Clear();
    _instance_buffer.Clear();
    _draw_material_indices_buffer.Clear();

    for (auto const& [key, model_matrices] : grouped)
    {
      auto const  mesh_views = mesh_manager.GetMeshData(key.tag);
      auto const& submesh    = mesh_views[key.sub_idx];

      _draw_material_indices_buffer.Append(submesh.material_index);

      auto const base_instance = static_cast<u32>(_instance_buffer.Size());
      _instance_buffer.Append(model_matrices);

      auto cmd = DrawIndirectCommand
      {
        .count          = submesh.index_count,
        .instance_count = static_cast<u32>(model_matrices.size()),
        .first_index    = submesh.index_offset,
        .base_vertex    = static_cast<i32>(submesh.vertex_offset),
        .base_instance  = base_instance
      };

      _indirect_buffer.Append(cmd);
    }

    UpdateCameraBuffer();

    _gbuffer_pipeline.Activate();

    ::glMultiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        nullptr,
        _indirect_buffer.Size(),
        0
    );
  }

}

