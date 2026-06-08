#include "ResourceLoaderService.hpp"
#include "Graphics/MeshData.hpp"
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
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/scalar_constants.hpp>
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

    static auto const extensions = std::array
    {
      ".gltf"s
      ".obj"s
      ".glb"s
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

    auto& mesh_mgr = ServiceLocator::GetInstance().Get<MeshManagerService>();
    if (mesh_mgr.IsModelLoaded(name))
    {
      log::Warn("Model with name {} already loaded !", name);
      return;
    }

    static constexpr auto k_assimp_flags =
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenUVCoords |
        aiProcess_FlipUVs;

    auto        importer = Assimp::Importer{};
    auto const* scene    = importer.ReadFile(abs_path.string(), k_assimp_flags);
    if ((not scene) or (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) or (not scene->mRootNode))
    {
      throw Exception{"Assimp error -> {}", std::string{importer.GetErrorString()}};
    }


    auto const material_base_index = static_cast<u32>(_global_materials.size());

    for (auto i : InRangeOf(0zu, scene->mNumMaterials))
    {
      auto* ai_mat = scene->mMaterials[i];
      auto  gm     = GlobalMaterial{};
      enum struct TextureTypeValue { Albedo, Normal, Roughness, Metalic };
      static constexpr std::array k_texture_type_map = 
      {
        std::pair{ aiTextureType_DIFFUSE,           TextureTypeValue::Albedo    },
        std::pair{ aiTextureType_NORMALS,           TextureTypeValue::Normal    },
        std::pair{ aiTextureType_DIFFUSE_ROUGHNESS, TextureTypeValue::Roughness },
        std::pair{ aiTextureType_METALNESS,         TextureTypeValue::Metalic   },
      };

      for (auto [type, slot] : k_texture_type_map)
      {
        auto ai_tex_path = aiString{};
        if (ai_mat->GetTexture(type, 0, &ai_tex_path) == aiReturn_SUCCESS and ai_tex_path.length > 0)
        {
          auto tex_abs_path = abs_path.parent_path() / ai_tex_path.C_Str();
          auto tag          = std::format("{}_{}", name, tex_abs_path.filename().string());
          LoadTextureTagged(tex_abs_path, tag);
          switch (slot) 
          {
            using enum TextureTypeValue;
            case Albedo    : gm.albedo_tag    = tag; break;
            case Normal    : gm.normal_tag    = tag; break;
            case Roughness : gm.roughness_tag = tag; break;
            case Metalic   : gm.metalic_tag   = tag; break;
            default : std::unreachable();
          }
        }
      }

      if (gm.albedo_tag.empty())
      {
        auto ai_bc_path = aiString{};
        if (ai_mat->GetTexture(aiTextureType_BASE_COLOR, 0, &ai_bc_path) == aiReturn_SUCCESS and ai_bc_path.length > 0)
        {
          auto tex_abs_path = abs_path.parent_path() / ai_bc_path.C_Str();
          auto tag          = std::format("{}_{}", name, tex_abs_path.filename().string());
          LoadTextureTagged(tex_abs_path, tag);
        }
      }

      _global_materials.push_back(std::move(gm));
      Application::GetInstance().GetEventBus().trigger<Event::MaterialLoaded>();
    }



    f32 const scale = [&]
    {
      auto local_min = glm::vec3( std::numeric_limits<f32>::max());
      auto local_max = glm::vec3(-std::numeric_limits<f32>::max());

      // Adjust local min max accordingly
      for (auto i : InRangeOf(0zu, scene->mNumMeshes))
      {
        auto const* mesh = scene->mMeshes[i];
        for (auto j : InRangeOf(0zu, mesh->mNumVertices))
        {
          auto& v = mesh->mVertices[j];
          local_min = glm::min(local_min, glm::vec3(v.x, v.y, v.z));
          local_max = glm::max(local_max, glm::vec3(v.x, v.y, v.z));
        }
      }

      glm::vec3 size       = local_max - local_min;
      auto      max_extent = glm::max(size.x, glm::max(size.y, size.z));
      static constexpr auto k_target_size = 3.0f;
      if (max_extent > 0.0f)
      {
        return k_target_size / max_extent;
      }

      return 1.0f;
    }();

    auto process_mesh = [&](aiMesh const* mesh, u32 global_mat_index) -> MeshData
    {
      auto md = MeshData{};
      md.material_index = global_mat_index;

      md.vertices.reserve(mesh->mNumVertices);
      for (auto i : InRangeOf(0zu, mesh->mNumVertices))
      { 
        auto v = VertexData{};
        if (mesh->HasPositions())
        {
          v.position = (glm::vec3(mesh->mVertices[i].x,
                                  mesh->mVertices[i].y,
                                  mesh->mVertices[i].z) * scale);
        }
        if (mesh->HasNormals())
        {
          v.normal = (glm::vec3(mesh->mNormals[i].x,
                                mesh->mNormals[i].y,
                                mesh->mNormals[i].z));
        }
        if (mesh->HasTextureCoords(0))
        {
          v.uv = glm::vec2(mesh->mTextureCoords[0][i].x,
                           mesh->mTextureCoords[0][i].y);
        }
        md.vertices.push_back(v);
      }

      md.indices.reserve(mesh->mNumFaces * 3);
      for (auto i : InRangeOf(0zu, mesh->mNumFaces))
      {
        auto const& ai_face = mesh->mFaces[i];
        for (auto j : InRangeOf(0zu, ai_face.mNumIndices))
        {
          md.indices.push_back(ai_face.mIndices[j]);
        }
      }

      return md;
    };

    auto all_meshes   = std::vector<MeshData>{};
    auto process_node = [&](this auto&& self, aiNode* node) -> void
    {
      for (auto i : InRangeOf(0zu, node->mNumMeshes))
      {
        u32         mesh_index       = node->mMeshes[i];
        auto const* ai_mesh          = scene->mMeshes[mesh_index];
        u32         global_mat_index = material_base_index + ai_mesh->mMaterialIndex;
        all_meshes.push_back(process_mesh(ai_mesh, global_mat_index));
      }

      for (auto i : InRangeOf(0zu, node->mNumChildren))
      {
        self(node->mChildren[i]);
      }
    };

    process_node(scene->mRootNode);

    mesh_mgr.LoadModelData(name, all_meshes);
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
      .format  = TextureFormat::Rgba8unorm,
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
