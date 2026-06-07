#include "MeshManagerService.hpp"
#include "Utils/Error.hpp"
#include <print>
#include <ranges>
#include <span>

namespace glr
{

  MeshManagerService::MeshManagerService()
    : _vertex_data(1024 * 1024, "Mesh Manager Vertex Buffer")
    , _index_data(1024 * 1024, "Mesh Manager Index Buffer")
    , _mesh_lookup{}
  {

  }

  auto MeshManagerService::LoadModel(std::string_view name, std::span<MeshData const> meshes) -> void
  {
    auto views = std::vector<MeshView>{};
    views.reserve(meshes.size());

    for (MeshData const& mesh : meshes)
    {
      auto vert_start = _vertex_data.Size();
      _vertex_data.Append(mesh.vertices);

      auto idx_start = _index_data.Size();
      _index_data.Append(mesh.indices);

      views.push_back(MeshView{
        .index_offset   = static_cast<std::uint32_t>(idx_start),
        .index_count    = static_cast<std::uint32_t>(mesh.indices.size()),
        .vertex_offset  = static_cast<std::uint32_t>(vert_start),
        .vertex_count   = static_cast<std::uint32_t>(mesh.vertices.size()),
        .material_index = mesh.material_index
      });
    }

    _mesh_lookup[std::string(name)] = std::move(views);
  }

  auto MeshManagerService::GetMeshData(std::string_view name) const -> std::span<MeshView const>
  {
    auto it = _mesh_lookup.find(name);
    Expect(it != _mesh_lookup.end(), "Mesh {} does not exist !!", name);
    return {it->second.data(), it->second.size()};
  }

  auto MeshManagerService::IsMeshLoaded(std::string const& name) const -> bool
  {
    return _mesh_lookup.contains(name);
  }

  auto MeshManagerService::GetModelNames() const -> std::vector<std::string_view>
  {
    return _mesh_lookup
      | std::views::transform([](auto const& e) -> std::string_view const {return e.first;}) 
      | std::ranges::to<std::vector>();
  }

  auto MeshManagerService::OnInit() -> void 
  {

  }

  auto MeshManagerService::OnUpdate()   -> void 
  {

  }

  auto MeshManagerService::OnShutdown() -> void 
  {

  }

}
