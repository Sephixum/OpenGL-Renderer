#include "ResourceLoaderService.hpp"
#include "Graphics/Model.hpp"
#include "Graphics/VertexData.hpp"
#include "ServiceLocator.hpp"
#include "MeshManagerService.hpp"
#include "ShaderManagerService.hpp"
#include "TextureManagerService.hpp"

#include "Core/Application.hpp"
#include "Core/Event.hpp"

#include "Graphics/Texture.hpp"
#include "Graphics/SamplerLibrary.hpp"


#include "Utils/InRangeOf.hpp"
#include "Utils/Utils.hpp"

#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/types.h>
#include <cctype>
#include <filesystem>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/glm.hpp>
#include <limits>
#include <ranges>
#include <stb_image.h>
#include <utility>

namespace fs = std::filesystem;

namespace glr
{
  
  auto ResourceLoaderService::OnInit() -> void
  {
    LoadAllAssetShaders();
    LoadAllAssetModels();
  }

  auto ResourceLoaderService::LoadModel(fs::path const& abs_path) -> void
  {
    if ((not fs::exists(abs_path)) or (not fs::is_regular_file(abs_path)))
    {
      log::Warn("Model file '{}' not found !", abs_path.string());
      return;
    }
    auto const tag = abs_path.stem().string();
    LoadModelFromFile(abs_path, tag);
  }

  auto ResourceLoaderService::LoadModelTagged(fs::path const& abs_path, std::string const& name) -> void
  {
    if ((not fs::exists(abs_path)) or (not fs::is_regular_file(abs_path)))
    {
      log::Warn("Model file '{}' not found !", abs_path.string());
      return;
    }
    LoadModelFromFile(abs_path, name);
  }

  auto ResourceLoaderService::LoadTexture(fs::path const& abs_path) -> void
  {
    if ((not fs::exists(abs_path)) or (not fs::is_regular_file(abs_path)))
    {
      log::Warn("Texture file '{}' not found !", abs_path.string());
      return;
    }
    auto tag = abs_path.stem().string();
    LoadTextureFromFile(abs_path, tag);
  }

  auto ResourceLoaderService::LoadTextureTagged(fs::path const& abs_path, std::string const& name) -> void
  {
    if ((not fs::exists(abs_path)) or (not fs::is_regular_file(abs_path)))
    {
      log::Warn("Texture file '{}' not found !", abs_path.string());
      return;
    }
    LoadTextureFromFile(abs_path, name);
  }

  auto ResourceLoaderService::LoadAllAssetModels() -> void
  {
    using namespace std::literals;
    namespace rng = std::ranges;
    namespace vws = std::views;

    [[maybe_unused]] static auto const extensions = std::array
    {
      ".gltf"sv,
      ".obj"sv,
      ".glb"sv
    };

    auto const models_root = fs::current_path() / "assets" / "models";
    if (not fs::exists(models_root))
    {
      log::Warn("Models root not found -> {}", models_root.string());
      return;
    }

    auto is_model_file = [](fs::directory_entry const& entry) static -> bool
    {
      if (not entry.is_regular_file()) return false;
      auto ext = entry.path().filename().extension().string();
      ext = ext | vws::transform([](unsigned char c){return std::tolower(c);}) | rng::to<std::string>();
      return ext == ".gltf" or ext == ".glb" or ext == ".obj";
    };

    auto entries = rng::subrange(fs::recursive_directory_iterator{models_root}, {});

    for (auto const& entry : entries | vws::filter(is_model_file))
    {
      LoadModel(entry.path());
    }
  }

  auto ResourceLoaderService::LoadModelFromFile(fs::path const& abs_path, std::string const& name) -> void
  {
    using namespace std::literals;
    static constexpr auto k_assimp_flags =
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenUVCoords |
        aiProcess_FlipUVs;

    auto& mesh_mgr = ServiceLocator::GetInstance().Get<MeshManagerService>();
    if (mesh_mgr.IsModelLoaded(name))
    {
      log::Warn("Model with name {} already loaded !", name);
      return;
    }

    auto        importer = Assimp::Importer{};
    auto const* ai_scene    = importer.ReadFile(abs_path.string(), k_assimp_flags);
    if ((not ai_scene) or (ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) or (not ai_scene->mRootNode))
    {

      throw Exception{"Assimp error -> {}", std::string{importer.GetErrorString()}};
    }

    auto model = ModelData{};

    auto const material_base_index = static_cast<u32>(_global_materials.size());

    auto load_texture_fn = [&](aiMaterial* mat, aiTextureType type) -> std::optional<std::string>
    {
      aiString ai_path;

      if (mat->GetTexture(type, 0, &ai_path) != aiReturn_SUCCESS)
      {
        return std::nullopt;
      }

      auto tex_abs_path =
        abs_path.parent_path() /
        ai_path.C_Str();

      auto tag =
        std::format(
            "{}_{}",
            name,
            tex_abs_path.filename().string());

      LoadTextureTagged(tex_abs_path, tag);
      return tag;
    };

    for (auto i : InRangeOf(0zu, ai_scene->mNumMaterials))
    {
      auto* ai_mat = ai_scene->mMaterials[i];

      auto gm = GlobalMaterial{};
      
      gm.albedo_tag =
          load_texture_fn(ai_mat, aiTextureType_BASE_COLOR)
              .value_or("");

      gm.normal_tag =
          load_texture_fn(ai_mat, aiTextureType_NORMALS)
              .value_or("");

      gm.roughness_tag =
          load_texture_fn(ai_mat, aiTextureType_DIFFUSE_ROUGHNESS)
              .value_or("");

      gm.metalic_tag =
          load_texture_fn(ai_mat, aiTextureType_METALNESS)
              .value_or("");

      // Fallback for OBJ files
      if (gm.albedo_tag.empty())
      {
        gm.albedo_tag =
            load_texture_fn(ai_mat, aiTextureType_DIFFUSE)
                .value_or("");
      }

      _global_materials.push_back(std::move(gm));

      Application::GetInstance()
          .GetEventBus()
          .trigger<Event::MaterialLoaded>();
    }

    auto process_mesh_fn = [] [[nodiscard]] (aiMesh const* ai_mesh, u32 global_material_index) static -> MeshData
    {
      auto md = MeshData{};
      md.material_index = global_material_index;
      md.vertices.reserve(ai_mesh->mNumVertices);
      md.indices.reserve(ai_mesh->mNumFaces * 3);

      // Vertex processing
      for (auto i : InRangeOf(0zu, ai_mesh->mNumVertices))
      {
        auto v = VertexData{};

        // Position
        if (ai_mesh->HasPositions())
        {
          v.position = 
          {
            ai_mesh->mVertices[i].x,
            ai_mesh->mVertices[i].y,
            ai_mesh->mVertices[i].z
          };

          md.bounds.min = glm::min(md.bounds.min, v.position);
          md.bounds.max = glm::max(md.bounds.max, v.position);
        }

        // Normal
        if (ai_mesh->HasNormals())
        {
          v.normal = 
          {
            ai_mesh->mNormals[i].x,
            ai_mesh->mNormals[i].y,
            ai_mesh->mNormals[i].z
          };
        }
        
        // UV
        if (ai_mesh->HasTextureCoords(0))
        {
          v.uv =
          {
            ai_mesh->mTextureCoords[0][i].x,
            ai_mesh->mTextureCoords[0][i].y,
          };
        }
        
        md.vertices.push_back(v);
      }
      
      // Index processing
      for (auto i : InRangeOf(0zu, ai_mesh->mNumFaces))
      {
        auto const& ai_face = ai_mesh->mFaces[i];
        for (auto j : InRangeOf(0zu, ai_face.mNumIndices))
        {
          md.indices.push_back(ai_face.mIndices[j]);
        }
      }

      md.vertices.shrink_to_fit();
      md.indices.shrink_to_fit();
      return md;
    };

    auto process_node_fn = [&] [[nodiscard]] (this auto&& self, aiNode const* ai_node) -> NodeData
    {
      auto nd = NodeData{};

      nd.name = ai_node->mName.C_Str();
      auto const& m = ai_node->mTransformation;

      nd.local_transform = 
      {
        m.a1, m.b1, m.c1, m.d1,
        m.a2, m.b2, m.c2, m.d2,
        m.a3, m.b3, m.c3, m.d3,
        m.a4, m.b4, m.c4, m.d4
      };

      for (auto i : InRangeOf(0zu, ai_node->mNumMeshes))
      {
        u32         mesh_index            = ai_node->mMeshes[i];
        auto const* ai_mesh               = ai_scene->mMeshes[mesh_index];
        u32         global_material_index = material_base_index + ai_mesh->mMaterialIndex;
        u32         runtime_mesh_index    = static_cast<u32>(model.meshes.size());
        model.meshes.push_back(process_mesh_fn(ai_mesh, global_material_index));
        nd.mesh_indices.push_back(runtime_mesh_index);

      }

      for (auto i : InRangeOf(0zu, ai_node->mNumChildren))
      {
        nd.children.push_back(self(ai_node->mChildren[i]));
      }

      return nd;
    };

    model.root = process_node_fn(ai_scene->mRootNode);

    f32 const scale = [&]()
    {
      glm::vec3 model_min(std::numeric_limits<float>::max());
      glm::vec3 model_max(std::numeric_limits<float>::lowest());

      for (auto const& mesh : model.meshes)
      {
          model_min = glm::min(model_min, mesh.bounds.min);
          model_max = glm::max(model_max, mesh.bounds.max);
      }

      glm::vec3 size = model_max - model_min;
      f32 max_extent = glm::max(size.x, std::max(size.y, size.z));
      constexpr f32 k_target_size = 3.0f;

      return (max_extent > 0.0f) ? k_target_size / max_extent : 1.0f;
    }();

    model.root.local_transform = glm::scale(glm::mat4(1.0f), glm::vec3(scale)) *
                                 model.root.local_transform;

    auto compute_node_bounds_fn = [&](auto&& self, NodeData const& node) -> AABB
    {
      AABB bounds;
      bounds.min = glm::vec3(std::numeric_limits<float>::max());
      bounds.max = glm::vec3(std::numeric_limits<float>::lowest());

      for (auto mesh_idx : node.mesh_indices)
      {
          auto mesh_bounds = model.meshes[mesh_idx].bounds;

          // Transform the AABB by node.local_transform
          glm::vec3 corners[8] = {
              {mesh_bounds.min.x, mesh_bounds.min.y, mesh_bounds.min.z},
              {mesh_bounds.max.x, mesh_bounds.min.y, mesh_bounds.min.z},
              {mesh_bounds.min.x, mesh_bounds.max.y, mesh_bounds.min.z},
              {mesh_bounds.min.x, mesh_bounds.min.y, mesh_bounds.max.z},
              {mesh_bounds.max.x, mesh_bounds.max.y, mesh_bounds.min.z},
              {mesh_bounds.min.x, mesh_bounds.max.y, mesh_bounds.max.z},
              {mesh_bounds.max.x, mesh_bounds.min.y, mesh_bounds.max.z},
              {mesh_bounds.max.x, mesh_bounds.max.y, mesh_bounds.max.z},
          };

          for (auto& c : corners)
          {
              c = glm::vec3(node.local_transform * glm::vec4(c, 1.0f));
              bounds.min = glm::min(bounds.min, c);
              bounds.max = glm::max(bounds.max, c);
          }
      }

      for (auto const& child : node.children)
      {
        auto child_bounds = self(self, child);
        bounds = AABB::MergeAABBs(bounds, child_bounds);
      }

      return bounds;
    };

    model.bounds = compute_node_bounds_fn(compute_node_bounds_fn, model.root);
    log::Info(
        "Model '{}' contains {} runtime meshes",
        name,
        model.meshes.size()
    );
    mesh_mgr.LoadModelData(name, model);
  }

  auto ResourceLoaderService::LoadTextureFromFile(fs::path const& abs_path, std::string const& tag) -> void
  {
    namespace rng = std::ranges;
    namespace vws = std::views;

    auto& tex_mgr = ServiceLocator::GetInstance().Get<TextureManagerService>();
    if (tex_mgr.IsTextureLoaded(tag))
    {
      log::Warn("Texture file '{}' already loaded !", abs_path.string());
      return;
    }

    auto extension = abs_path.extension().string();
    extension = extension | vws::transform(::tolower) | rng::to<std::string>();
    if ((extension == ".hdr") or (extension == ".exr"))
    {
      log::Warn("Texture {} of type '{}' not supported !", abs_path.string(), extension);
      return;
    }

    auto width  = 0;
    auto height = 0;

    auto* pixels_data = stbi_load(abs_path.string().c_str(), &width, &height, nullptr, 4);
    if (not pixels_data)
    {
      log::Warn("Texture {} could not be loaded by stb_image: \n\t{}", abs_path.string(), stbi_failure_reason());
      return;
    }

    auto info = Texture2DCreateInfo
    {
      .width   = static_cast<u32>(width),
      .height  = static_cast<u32>(height),
      .format  = TextureFormatType::Rgba8unorm,
      .sampler = SamplerLibrary::ModelMipmapAniso16x(),
      .data    = std::vector<std::byte>{reinterpret_cast<std::byte*>(pixels_data),
                                        reinterpret_cast<std::byte*>(pixels_data) + width * height * 4},
      .mipmap_levels = 10
    };

    stbi_image_free(pixels_data);
    tex_mgr.AddTexture2D(info, tag);
  }

  auto ResourceLoaderService::LoadAllAssetShaders() -> void
  {
    using namespace std::literals;
    namespace rng = std::ranges;
    namespace vws = std::views;

    auto&      shader_mgr   = ServiceLocator::GetInstance().Get<ShaderManagerService>();
    auto const shaders_root = fs::current_path() / "assets" / "shaders";
    log::Info("shaders root :{}", shaders_root.string());

    if (not fs::exists(shaders_root)) 
    {
      throw Exception{"Shaders root not found: {}", shaders_root.string()};
    }

    auto is_shader_file = [](fs::directory_entry const& entry) static -> bool
    {
      if (not entry.is_regular_file()) return false;
      auto ext = entry.path().filename().extension().string();
      ext = ext | vws::transform([](unsigned char c){return std::tolower(c);}) | rng::to<std::string>();
      return ext == ".vert" or ext == ".frag" or ext == ".comp";
    };

    auto entries = rng::subrange(fs::recursive_directory_iterator{shaders_root}, {});


    for (auto const& entry : entries | vws::filter(is_shader_file))
    {
      shader_mgr.LoadShader(entry.path());
    }
  }

  auto ResourceLoaderService::LoadShader(fs::path const& abs_path) -> void
  {
    (void)abs_path;
  }

  auto ResourceLoaderService::LoadShaderTagged(fs::path const& abs_path, std::string const& name) -> void
  {
    (void)abs_path;
    (void)name;
  }

  auto ResourceLoaderService::GetGlobalMaterial() -> std::span<GlobalMaterial>
  {
    return _global_materials;
  }


}
