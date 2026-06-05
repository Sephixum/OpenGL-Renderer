#include "ResourceLoaderService.hpp"

#include "ServiceLocator.hpp"
#include "MeshManagerService.hpp"
#include "ShaderManagerService.hpp"

#include "Utils/Exception.hpp"

#include <algorithm>
#include <assimp/scene.h>
#include <cctype>
#include <filesystem>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/ext/quaternion_geometric.hpp>
#include <limits>
#include <set>


namespace glr
{

  namespace fs = std::filesystem;

  auto ResourceLoaderService::OnInit() -> void
  {
    {
      auto& shader_service = ServiceLocator::GetInstance().Get<ShaderManagerService>();
      auto  shaders_root   = fs::current_path() / "assets" / "shaders";
      if (not fs::exists(shaders_root)) return;

      for (auto const& entry : fs::directory_iterator(shaders_root))
      {
        if (not entry.is_directory()) continue;
        auto shader_name = entry.path().filename().string();
        shader_service.LoadShader(shader_name);
      }
    }

    LoadAllAssetModels();
  }

  auto ResourceLoaderService::OnUpdate() -> void 
  {

  }

  auto ResourceLoaderService::OnShutdown() -> void 
  {

  }

  auto ResourceLoaderService::LoadAllAssetModels() -> void
  {
    namespace rng = std::ranges;
    namespace vws = std::views;
    static std::set<std::string> const extensions = {".gltf", ".obj", ".glb"};
    static fs::path              const models_root  = fs::current_path() / "assets" / "models";
    if (not fs::exists(models_root)) return;

    auto is_model_file = [](const fs::directory_entry& entry) -> bool 
    {
        if (!entry.is_regular_file()) return false;
        std::string ext = entry.path().extension().string();
        rng::transform(ext, ext.begin(), ::tolower);
        return rng::find(extensions, ext) != extensions.end();
    };

    auto entries = rng::subrange(fs::recursive_directory_iterator(models_root),
                                 fs::recursive_directory_iterator{});

    for ( auto const& entry : entries | vws::filter(is_model_file))
    {
      LoadModel(entry.path());
    }

  }

  auto ResourceLoaderService::LoadModel(fs::path const& absolute_path_dir) -> void
  {
    if ((not fs::exists(absolute_path_dir)) or (not fs::is_regular_file(absolute_path_dir)))
    {
      return;
    }

    auto tag = absolute_path_dir.stem().string();
    LoadModelFromFile(absolute_path_dir, tag);
  }

  auto ResourceLoaderService::LoadModelFromFile(std::filesystem::path const& path, std::string const& name) -> void
  {
    static constexpr auto range = std::views::iota;
    auto& mesh_manager = ServiceLocator::GetInstance().Get<MeshManagerService>();
    if (mesh_manager.IsMeshLoaded(name)) return;

    fs::path model_dir = fs::current_path() / "assets" / "models" / name;

    static constexpr auto const flags = 
                aiProcess_Triangulate |
                aiProcess_GenNormals |
                aiProcess_JoinIdenticalVertices |
                aiProcess_FlipUVs;   // glTF UV orientation fix

    auto        importer = Assimp::Importer{};
    auto const* scene    = importer.ReadFile(path.string(), flags);
    if ((not scene) or (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) or (not scene->mRootNode))
    {
      throw Exception{"Assimp error -> {}", std::string(importer.GetErrorString())};
    }

    
    auto local_min = glm::vec3(std::numeric_limits<float>::min());
    auto local_max = glm::vec3(-std::numeric_limits<float>::max());
    for (auto m : range(0zu, scene->mNumMeshes))
    {
      auto const* mesh = scene->mMeshes[m];
      for (auto v : range(0zu, mesh->mNumVertices))
      {
        auto vert = mesh->mVertices[v];
        local_min = glm::min(local_min, glm::vec3(vert.x, vert.y, vert.z));
        local_max = glm::max(local_max, glm::vec3(vert.x, vert.y, vert.z));
      }
    }

    glm::vec3       size        = local_max - local_min;
    float           max_extent  = glm::max(size.x, glm::max(size.y, size.z));
    float           scale       = 1.0f;
    constexpr auto  k_target_size = 3.0f;

    if (max_extent > 0.0f)
    {
      scale = k_target_size / max_extent;
    }

    auto process_mesh_fn = [&scale](const aiMesh* mesh) -> MeshData 
    {
      static constexpr auto range = std::views::iota;
      auto mesh_data = MeshData{};

      mesh_data.vertices.reserve(mesh->mNumVertices);
      for (auto i : range(0zu, mesh->mNumVertices)) {
        auto v = VertexData{};
        
        if (mesh->HasPositions())
        {
          v.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z) * scale;
        }
        if (mesh->HasNormals())
        {
          v.normal = glm::normalize(glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
        }
        if (mesh->mTextureCoords[0])
        {
          v.uv = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }

        mesh_data.vertices.push_back(v);
      }

      mesh_data.indices.reserve(mesh->mNumFaces * 3);
      for (auto i : range(0zu, mesh->mNumFaces))
      {
        aiFace const& face = mesh->mFaces[i];
        for (auto j : range(0zu, face.mNumIndices))
        {
          mesh_data.indices.push_back(face.mIndices[j]);
        }
      }

      return mesh_data;
    };

    
    auto process_node_fn = [&](this auto&& self, aiNode* node) -> std::vector<MeshData>
    {
      static constexpr auto range = std::views::iota;
      auto meshes = std::vector<MeshData>{};

      for (auto i : range(0zu, node->mNumMeshes)) 
      {
        auto* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(process_mesh_fn(mesh));
      }

      for (auto i : range(0zu, node->mNumChildren)) 
      {
        auto child_meshes = self(node->mChildren[i]);
        meshes.insert_range(meshes.end(), child_meshes);
      }

      return meshes;
    };

    std::vector<MeshData> allMeshes = process_node_fn(scene->mRootNode);
    mesh_manager.LoadMesh(name, allMeshes);
  }

}
