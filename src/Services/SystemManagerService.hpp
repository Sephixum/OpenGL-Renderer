#pragma once

#include "IService.hpp"
#include "Systems/ISystem.hpp"
#include <concepts>
#include <memory>
#include <typeindex>
#include <map>

namespace glr
{

  class SystemManagerService : public IService
  {
    public:
      virtual auto OnInit()     -> void override;
      virtual auto OnUpdate()   -> void override;
      virtual auto OnShutdown() -> void override;

      template <std::derived_from<ISystem> T, typename... Args>
      auto AddSystem(Args&&... args);

    private:
      std::map<std::type_index, std::unique_ptr<ISystem>> m_systems;
  };


  template <std::derived_from<ISystem> T, typename... Args>
  auto SystemManagerService::AddSystem(Args&&... args)
  {
    static std::type_index const s_type_index = typeid(T);
    m_systems[s_type_index] = std::make_unique<T>(std::forward<Args>(args)...);
  }

}
