#include "MeshManagerService.hpp"
#include "Graphics/Model.hpp"
#include "Graphics/Primitive.hpp"
#include "Utils/Error.hpp"
#include "Utils/Log.hpp"
#include <print>
#include <ranges>
#include <span>

namespace glr
{

  MeshManagerService::MeshManagerService()
    : _vertex_data(1024 * 1024, "Mesh Manager Vertex Buffer")
    , _index_data(1024 * 1024, "Mesh Manager Index Buffer")
    , _model_lookup{}
  {

  }

  auto MeshManagerService::OnInit() -> void
  {
    LoadModelData("Cube", Primitive::Cube());
    LoadModelData("Sphere", Primitive::Sphere());
    LoadModelData("Cylinder", Primitive::Cylinder());
  }

  auto MeshManagerService::LoadModelData(std::string_view name, ModelData const& model) -> void
  {
    if (IsModelLoaded(name)) 
    {
      log::Warn("Modle with name {} already loaded !", name);
      return;
    }

    auto runtime_model = ModelView{};
    runtime_model.meshes.reserve(model.meshes.size());

    for (MeshData const& mesh : model.meshes)
    {
      auto const vert_start = _vertex_data.Size();
      _vertex_data.Append(mesh.vertices);

      auto const index_start = _index_data.Size();
      _index_data.Append(mesh.indices);

      runtime_model.meshes.push_back(MeshView{
          .index_offset   = static_cast<u32>(index_start),
          .index_count    = static_cast<u32>(mesh.indices.size()),
          .vertex_offset  = static_cast<u32>(vert_start),
          .vertex_count   = static_cast<u32>(mesh.vertices.size()),
          .material_index = mesh.material_index
      });
    }

    runtime_model.root               = model.root;
    runtime_model.bounds             = model.bounds;
    _model_lookup[std::string(name)] = std::move(runtime_model);
  }

  auto MeshManagerService::GetModel(std::string_view name) const -> ModelView const&
  {
    auto iter = _model_lookup.find(std::string(name));
    Expect(iter != _model_lookup.end(), "Model name {} not found !", name);
    return iter->second;
  }

  auto MeshManagerService::TryGetModel(std::string_view name) const -> ModelView const*
  {
    auto iter = _model_lookup.find(std::string(name));
    if (iter == _model_lookup.end())
    {
      return nullptr;
    }
    return &(iter->second);
  }

  auto MeshManagerService::IsModelLoaded(std::string_view name) const -> bool
  {
    return _model_lookup.contains(name);
  }

  auto MeshManagerService::GetModelNames() const -> std::vector<std::string>
  {
    return _model_lookup
      | std::views::transform([](auto const& e) -> std::string {return e.first;}) 
      | std::ranges::to<std::vector>();
  }

}
