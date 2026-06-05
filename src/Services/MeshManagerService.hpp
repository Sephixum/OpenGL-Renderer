#pragma once

#include "IService.hpp"
#include "Graphics/VertexData.hpp"
#include "Graphics/Buffer.hpp"
#include "Graphics/MeshView.hpp"
#include "Graphics/MeshData.hpp"
#include "Utils/StringMap.hpp"
#include <vector>

namespace glr
{

  class MeshManagerService : public IService
  {
    DynamicPersistantBuffer<VertexData>    _vertex_data;
    DynamicPersistantBuffer<std::uint32_t> _index_data;
    StringMap<std::vector<MeshView>>       _mesh_lookup;

    public:
      MeshManagerService();

      auto LoadMesh(std::string_view name, std::span<MeshData const> meshes) -> void;

      virtual auto OnInit()     -> void override;
      virtual auto OnUpdate()   -> void override;
      virtual auto OnShutdown() -> void override;

      [[nodiscard]] auto GetMeshData(std::string_view name) const -> std::span<MeshView const>;
      [[nodiscard]] auto GetVertexBuffer() -> DynamicPersistantBuffer<VertexData>&    { return _vertex_data; }
      [[nodiscard]] auto GetIndexBuffer()  -> DynamicPersistantBuffer<std::uint32_t>& { return _index_data;  }
      [[nodiscard]] auto GetVertexBuffer() const -> DynamicPersistantBuffer<VertexData> const&    { return _vertex_data; }
      [[nodiscard]] auto GetIndexBuffer()  const -> DynamicPersistantBuffer<std::uint32_t> const& { return _index_data;  }
      [[nodiscard]] auto IsMeshLoaded(std::string const& name) const -> bool;
      [[nodiscard]] auto GetModelNames() const -> std::vector<std::string_view>;
  };

}
