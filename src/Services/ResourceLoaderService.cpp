#include "ResourceLoaderService.hpp"
#include "Graphics/ShaderProgram.hpp"
#include "MeshManagerService.hpp"
#include "ServiceLocator.hpp"
#include "ShaderManagerService.hpp"
#include "Utils/Error.hpp"
#include "Utils/Exception.hpp"

#include <assimp/scene.h>
#include <filesystem>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>


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

    {
      auto models_root = fs::current_path() / "assets" / "models";
      if (not fs::exists(models_root)) return;

      for (auto const& entry : fs::directory_iterator(models_root))
      {
        if (not entry.is_directory()) continue;
        auto model_name = entry.path().filename().string();
        LoadModel(model_name);
      }
    }
  }

  auto ResourceLoaderService::OnUpdate() -> void 
  {

  }

  auto ResourceLoaderService::OnShutdown() -> void 
  {

  }

  auto ResourceLoaderService::LoadModel(std::string const& name) -> void
  {
    auto& mesh_manager = ServiceLocator::GetInstance().Get<MeshManagerService>();
    if (mesh_manager.IsMeshLoaded(name)) return;

    fs::path model_dir = fs::current_path() / "assets" / "models" / name;
    static constexpr auto const k_extensions = std::array{
      ".gltf", ".obj"
    };

    auto model_file = fs::path{};
    for (auto const& extension : k_extensions)
    {
      auto candidate = model_dir / (name + extension);
      if (fs::exists(candidate))
      {
        model_file = candidate;
        break;
      }
    }

    if (model_file.empty()) return;

    static constexpr auto const flags = 
                aiProcess_Triangulate |
                aiProcess_GenNormals |
                aiProcess_JoinIdenticalVertices |
                aiProcess_FlipUVs;   // glTF UV orientation fix

    auto        importer = Assimp::Importer{};
    auto const* scene    = importer.ReadFile(model_file, flags);

    if ((not scene) or (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) or (not scene->mRootNode))
    {
      throw Exception{"Assimp error -> {}", std::string(importer.GetErrorString())};
    }

    auto process_mesh_fn = [](const aiMesh* mesh) static -> MeshData 
    {
      static constexpr auto range = std::views::iota;
      auto mesh_data = MeshData{};

      mesh_data.vertices.reserve(mesh->mNumVertices);
      for (auto i : range(0zu, mesh->mNumVertices)) {
        auto v = VertexData{};
        
        if (mesh->HasPositions())
        {
          v.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        }
        if (mesh->HasNormals())
        {
          v.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
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
