#pragma once

#include "IService.hpp"
#include "Graphics/VertexData.hpp"
#include "Graphics/Buffer.hpp"
#include "Graphics/Model.hpp"
#include "Utils/StringMap.hpp"
#include <vector>

namespace glr
{

  class MeshManagerService : public IService
  {
    DynamicPersistantBuffer<VertexData>    _vertex_data;
    DynamicPersistantBuffer<std::uint32_t> _index_data;
    StringMap<ModelView>                   _model_lookup;

    public:
      MeshManagerService();

      virtual auto OnInit()     -> void override {}
      virtual auto OnUpdate()   -> void override {}
      virtual auto OnShutdown() -> void override {}

      auto LoadModelData(std::string_view name, ModelData const& model) -> void;

      [[nodiscard]] auto GetModel(std::string_view name) const -> ModelView const&;
      [[nodiscard]] auto TryGetModel(std::string_view name) const -> ModelView const*;
      [[nodiscard]] auto GetVertexBuffer() -> DynamicPersistantBuffer<VertexData>&    { return _vertex_data; }
      [[nodiscard]] auto GetIndexBuffer()  -> DynamicPersistantBuffer<std::uint32_t>& { return _index_data;  }
      [[nodiscard]] auto GetVertexBuffer() const -> DynamicPersistantBuffer<VertexData> const&    { return _vertex_data; }
      [[nodiscard]] auto GetIndexBuffer()  const -> DynamicPersistantBuffer<std::uint32_t> const& { return _index_data;  }
      [[nodiscard]] auto IsModelLoaded(std::string_view name) const -> bool;
      [[nodiscard]] auto GetModelNames() const -> std::vector<std::string>;
  };

}
