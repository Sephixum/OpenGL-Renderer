#pragma once

#include "IService.hpp"
#include "Graphics/ShaderProgram.hpp"
#include <unordered_map>
#include <string>

namespace glr 
{

  class ResourceLoaderService : public IService 
  {
    auto LoadModel(std::string const& name)  -> void;

    public:
      auto OnInit()     -> void override;
      auto OnUpdate()   -> void override;
      auto OnShutdown() -> void override;
  };

}
