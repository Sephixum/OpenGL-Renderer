#include "Components/Components.hpp"

#include "RenderingSystem.hpp"

#include "Core/Application.hpp"
#include "Core/Event.hpp"
#include "Graphics/FrameBuffer.hpp"
#include "Graphics/IGLResource.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/VertexArray.hpp"
#include "Graphics/GraphicsPipeline.hpp"
#include "Graphics/Buffer.hpp"

#include "Services/MeshManagerService.hpp"
#include "Services/ServiceLocator.hpp"
#include "Services/ShaderManagerService.hpp"
#include "Services/SceneManagerService.hpp"
#include "Services/ResourceLoaderService.hpp"
#include "Services/TextureManagerService.hpp"
#include "Services/WindowService.hpp"

#include "Utils/InRangeOf.hpp"

#include <functional>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/matrix.hpp>
#include <iterator>
#include <span>
#include <unordered_map>
#include <entt/entt.hpp>
#include <vector>

namespace helper
{

  [[nodiscard]] auto CreateGBufferFrameBufferInfo()                                                   -> glr::FramebufferCreateInfo;
  [[nodiscard]] auto CreateLightingFrameBufferInfo()                                                  -> glr::FramebufferCreateInfo;
  [[nodiscard]] auto FlattenModelMeshes(glr::NodeData const& node, glm::mat4 const& parent_transform) -> std::vector<glr::MeshFlattened>;

}

namespace glr
{

  RenderingSystem::RenderingSystem()
    :
    _geometry
    {
      .vao                          = {"Geometry VertexArray"},
      .pipeline                     = {"Gbuffer GraphicsPipeline"},
      .frame_buffer                 = {helper::CreateGBufferFrameBufferInfo(), "Gbuffer FrameBuffer"},
      .indirect_buffer              = {10, "Indirect Command Buffer"},
      .instance_buffer              = {10, "Instance Data Buffer"},
      .camera_buffer                = {1, "Camera Data Buffer"},
      .material_buffer              = {10, "GlobalMaterials Buffer"},
      .draw_material_indices_buffer = {256, "DrawMaterialIndices"}     
    }
    ,
    _lighting
    {
      .vao                             = {"Lighting VertexArray"},
      .pipeline                        = {"Lighting GraphicsPipeline"},
      .frame_buffer                    = {helper::CreateLightingFrameBufferInfo(), "Lighting FrameBuffer"},
      .geometry_bindless_handle_buffer = {4, "Lighting GbufferHandles"}
    }
  {
    glEnable(GL_DEPTH_TEST);

    auto& shader_manager = ServiceLocator::GetInstance().Get<ShaderManagerService>();
    auto& mesh_manager   = ServiceLocator::GetInstance().Get<MeshManagerService>();

    auto& gbuffer_vert_shader = shader_manager.GetVertexShader("gbuffer");
    auto& gbuffer_frag_shader = shader_manager.GetFragmentShader("gbuffer");

    _geometry.pipeline
      .SetVertexShader(gbuffer_vert_shader)
      .SetFragmentShader(gbuffer_frag_shader);

    _geometry.vao.BuildSettings()
      .BindAs<BufferType::ShaderStorage>(mesh_manager.GetVertexBuffer(), 0)
      .BindAs<BufferType::ShaderStorage>(_geometry.camera_buffer, 1)
      .BindAs<BufferType::ShaderStorage>(_geometry.instance_buffer, 2)
      .BindAs<BufferType::ShaderStorage>(_geometry.draw_material_indices_buffer, 3)
      .BindAs<BufferType::ShaderStorage>(_geometry.material_buffer, 4)
      .BindAs<BufferType::Index>(mesh_manager.GetIndexBuffer())
      .BindAs<BufferType::DrawIndirect>(_geometry.indirect_buffer)
      .Apply()
      .Activate();


    auto& lighting_vert_shader = shader_manager.GetVertexShader("lighting");
    auto& lighting_frag_shader = shader_manager.GetFragmentShader("lighting");

    _lighting.pipeline
      .SetVertexShader(lighting_vert_shader)
      .SetFragmentShader(lighting_frag_shader);

    _lighting.vao.BuildSettings()
      .BindAs<BufferType::ShaderStorage>(_lighting.geometry_bindless_handle_buffer, 5)
      .Apply()
      .Activate();

    BuildGlobalMaterials();
    _material_buffer_rebuild_sink = Application::GetInstance().GetEventBus().sink<Event::MaterialLoaded>().connect<&RenderingSystem::BuildGlobalMaterials>(this);
  }

  auto RenderingSystem::Invoke() -> void
  {
    if (_state.gbuffer_pass) RenderGBufferPass();
    RenderLightingPass();
  }

  auto RenderingSystem::BuildGlobalMaterials() -> void
  {
    auto& res_ldr = ServiceLocator::GetInstance().Get<ResourceLoaderService>();
    auto& tex_mgr = ServiceLocator::GetInstance().Get<TextureManagerService>();
    _geometry.material_buffer.Clear();

    for (auto const& mat : res_ldr.GetGlobalMaterial())
    {
      auto gpu = GPUMaterialData{};
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
      _geometry.material_buffer.Append(gpu);
    }
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
    _geometry.camera_buffer.WriteData(std::as_bytes(std::span{&camera_data, 1}));
  }

  auto RenderingSystem::RenderGBufferPass() -> void
  {
    [[maybe_unused]] auto& res_ldr         = ServiceLocator::GetInstance().Get<ResourceLoaderService>();
    [[maybe_unused]] auto& mesh_manager    = ServiceLocator::GetInstance().Get<MeshManagerService>();
    [[maybe_unused]] auto& texture_manager = ServiceLocator::GetInstance().Get<TextureManagerService>();
    [[maybe_unused]] auto& window          = ServiceLocator::GetInstance().Get<WindowService>();
    auto& reg             = ServiceLocator::GetInstance().Get<SceneManagerService>().GetActiveScene().registry;

    _geometry.frame_buffer.Bind();
    glViewport(0, 0, window.GetWidth(), window.GetHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    auto view = reg.view<Component::MeshAsset, Component::Transform>();

    if (std::ranges::distance(view.each()) == 0)
    {
      return;
    }

    struct SubMeshKey 
    {
      std::string tag     = {};
      u64         sub_idx = {};
      auto operator<=>(SubMeshKey const&) const = default;
    };

    auto sub_mesh_key_hash_fn = [](SubMeshKey const& k) static -> std::size_t
    {
      auto h1 = std::hash<std::string>{}(k.tag);
      auto h2 = std::hash<u64>{}(k.sub_idx);
      return h1 ^ (h2 << 1);
    };

    auto grouped = std::unordered_map
      <
        SubMeshKey,                       // DataType
        std::vector<MeshInstanceData>,    // Value
        decltype(sub_mesh_key_hash_fn)    // HashFunction
      >
      {0, sub_mesh_key_hash_fn};

    for (auto [e, e_mesh, e_transform] : view.each())
    {
      if (not mesh_manager.IsModelLoaded(e_mesh.mesh_tag))
      {
        continue;
      }

      auto const& model_view   = mesh_manager.GetModel(e_mesh.mesh_tag);
      auto const  model_matrix = e_transform.GetMatrix();

      auto const flattened_meshes = helper::FlattenModelMeshes(model_view.root, model_matrix);

      log::Info(
          "Flattened mesh count = {}",
          flattened_meshes.size());

      for (auto const& f : flattened_meshes)
      {
        grouped[{e_mesh.mesh_tag, f.mesh_index}].push_back({f.world_transform});
      }

    }

    if (grouped.empty())
    {
      return;
    }

    _geometry.indirect_buffer.Clear();
    _geometry.instance_buffer.Clear();
    _geometry.draw_material_indices_buffer.Clear();

    for (auto const& [key, model_matrices] : grouped)
    {
      auto const  model_view = mesh_manager.GetModel(key.tag);
      auto const& submesh    = model_view.meshes[key.sub_idx];

      _geometry.draw_material_indices_buffer.Append(submesh.material_index);

      auto const base_instance = static_cast<u32>(_geometry.instance_buffer.Size());
      _geometry.instance_buffer.Append(model_matrices);

      auto cmd = DrawIndirectCommand
      {
        .count          = submesh.index_count,
        .instance_count = static_cast<u32>(model_matrices.size()),
        .first_index    = submesh.index_offset,
        .base_vertex    = static_cast<i32>(submesh.vertex_offset),
        .base_instance  = base_instance
      };

      _geometry.indirect_buffer.Append(cmd);
    }

    UpdateCameraBuffer();

    _geometry.pipeline.Activate();

    _geometry.vao.BuildSettings()
      .BindAs<BufferType::ShaderStorage>(mesh_manager.GetVertexBuffer(), 0)
      .BindAs<BufferType::ShaderStorage>(_geometry.camera_buffer, 1)
      .BindAs<BufferType::ShaderStorage>(_geometry.instance_buffer, 2)
      .BindAs<BufferType::ShaderStorage>(_geometry.draw_material_indices_buffer, 3)
      .BindAs<BufferType::ShaderStorage>(_geometry.material_buffer, 4)
      .BindAs<BufferType::Index>(mesh_manager.GetIndexBuffer())
      .BindAs<BufferType::DrawIndirect>(_geometry.indirect_buffer)
      .Apply()
      .Activate();

    ::glMultiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        nullptr,
        _geometry.indirect_buffer.Size(),
        0
    );

    _geometry.frame_buffer.UnBind();


  }

  auto RenderingSystem::RenderLightingPass() -> void
  {
    auto& window = glr::ServiceLocator::GetInstance().Get<glr::WindowService>();

    _lighting.frame_buffer.Bind();
    glViewport(0, 0, window.GetWidth(), window.GetHeight());
    glClear(GL_COLOR_BUFFER_BIT);

    auto* albedo   = _geometry.frame_buffer.TryGetAttachment(FramebufferSlotType::Albedo);
    auto* normal   = _geometry.frame_buffer.TryGetAttachment(FramebufferSlotType::Normal);
    auto* position = _geometry.frame_buffer.TryGetAttachment(FramebufferSlotType::Position);
    auto* material = _geometry.frame_buffer.TryGetAttachment(FramebufferSlotType::Material);

    _lighting.geometry_bindless_handle_buffer.Clear();

    _lighting.geometry_bindless_handle_buffer.Append(albedo   ? albedo->GetBindlessHandle()   : 0);
    _lighting.geometry_bindless_handle_buffer.Append(normal   ? normal->GetBindlessHandle()   : 0);
    _lighting.geometry_bindless_handle_buffer.Append(position ? position->GetBindlessHandle() : 0);
    _lighting.geometry_bindless_handle_buffer.Append(material ? material->GetBindlessHandle() : 0);


    _lighting.vao.BuildSettings()
      .BindAs<BufferType::ShaderStorage>(_lighting.geometry_bindless_handle_buffer, 5)
      .Apply()
      .Activate();
    
    _lighting.pipeline.Activate();

    ::glDrawArrays(GL_TRIANGLES, 0, 3); // dummy draw call
    
    _lighting.frame_buffer.UnBind();

    // Before the blit, set which attachment to read from
    glNamedFramebufferReadBuffer(_lighting.frame_buffer.GetID(), GL_COLOR_ATTACHMENT0); // albedo
                                                                                        //
    // Then blit
    glBlitNamedFramebuffer(_lighting.frame_buffer.GetID(), 0,
                            0, 0, window.GetWidth(), window.GetHeight(),
                            0, 0, window.GetWidth(), window.GetHeight(),
                            GL_COLOR_BUFFER_BIT,
                            GL_NEAREST);

  }

}

auto helper::CreateGBufferFrameBufferInfo() -> glr::FramebufferCreateInfo
{
  auto&      window   = glr::ServiceLocator::GetInstance().Get<glr::WindowService>();
  auto const window_w = window.GetWidth();
  auto const window_h = window.GetHeight();
  auto       info     = glr::FramebufferCreateInfo{};

  info.width          = window_w;
  info.height         = window_h;

  info.color_attachments[0] = glr::FramebufferAttachmentCreateInfo{
    .format = glr::TextureFormatType::Rgba8unorm,
    .width  = 0,
    .height = 0,
    .slot   = glr::FramebufferSlotType::Albedo
  };

  info.color_attachments[1] = glr::FramebufferAttachmentCreateInfo{
    .format = glr::TextureFormatType::Rgba16f,
    .width  = 0,
    .height = 0,
    .slot   = glr::FramebufferSlotType::Normal
  };

  info.color_attachments[2] = glr::FramebufferAttachmentCreateInfo{
    .format = glr::TextureFormatType::Rgba32f,
    .width  = 0,
    .height = 0,
    .slot   = glr::FramebufferSlotType::Position
  };

  info.color_attachments[3] = glr::FramebufferAttachmentCreateInfo{
    .format = glr::TextureFormatType::Rg8unorm,
    .width  = 0,
    .height = 0,
    .slot   = glr::FramebufferSlotType::Material
  };

  info.use_depthstencil = true;

  return info;
};

auto helper::CreateLightingFrameBufferInfo() -> glr::FramebufferCreateInfo
{
  auto& window = glr::ServiceLocator::GetInstance().Get<glr::WindowService>();
  auto  info   = glr::FramebufferCreateInfo{};
  info.width   = window.GetWidth();
  info.height  = window.GetHeight();
  info.color_attachments[0] = glr::FramebufferAttachmentCreateInfo{
    .format = glr::TextureFormatType::Rgba8unorm,
    .width  = 0,
    .height = 0,
    .slot   = glr::FramebufferSlotType::HDRColor
  };
  info.use_depthstencil = false;
  return info;
}


auto helper::FlattenModelMeshes(glr::NodeData const& root, glm::mat4 const& root_transform) -> std::vector<glr::MeshFlattened>
{
  auto result = std::vector<glr::MeshFlattened>{};

  auto recurse_fn = [&result](this auto&& self, glr::NodeData const& node, glm::mat4 const& parent_transform) -> void
  {
    auto const world_transform = parent_transform * node.local_transform;

    for (glr::u32 mesh_index : node.mesh_indices)
    {
      result.push_back(glr::MeshFlattened{
          .mesh_index      = mesh_index,
          .world_transform = world_transform
      });
    }

    for (auto const& child : node.children)
    {
      self(child, world_transform);
    }
  };

  recurse_fn(root, root_transform);

  return result;
}

