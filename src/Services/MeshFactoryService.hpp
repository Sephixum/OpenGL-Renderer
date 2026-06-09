#pragma once

#include "Services/IService.hpp"
#include "Graphics/MeshData.hpp"

namespace glr
{

  class MeshFactoryService : public IService
  {
    public:
      virtual auto OnInit()     -> void override {}
      virtual auto OnUpdate()   -> void override {}
      virtual auto OnShutdown() -> void override {}
  };

}
