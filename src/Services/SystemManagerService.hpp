#pragma once

#include "IService.hpp"
#include "Systems/ISystem.hpp"
#include "Utils/Error.hpp"
#include <concepts>
#include <memory>
#include <typeindex>
#include <flat_map>

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

      template <std::derived_from<ISystem> T>
      auto GetSystem() -> T&;

      template <std::derived_from<ISystem> T>
      auto TryGetSystem() -> T*;

    private:
      std::flat_map<std::type_index, std::unique_ptr<ISystem>> _systems;
  };


  template <std::derived_from<ISystem> T, typename... Args>
  auto SystemManagerService::AddSystem(Args&&... args)
  {
    static std::type_index const s_type_index = typeid(T);
    _systems[s_type_index] = std::make_unique<T>(std::forward<Args>(args)...);
  }

  template <std::derived_from<ISystem> T>
  auto SystemManagerService::GetSystem() -> T&
  {
    static std::type_index const s_type_index = typeid(T);
    auto it = _systems.find(s_type_index);
    Expect(it != _systems.end(), "System with mangled name {} not found!", s_type_index.name());
    return *it->second;
  }

  template <std::derived_from<ISystem> T>
  auto SystemManagerService::TryGetSystem() -> T*
  {
    static std::type_index const s_type_index = typeid(T);
    auto it = _systems.find(s_type_index);

    if(it == _systems.end())
    {
      return nullptr;
    }

    return it->second.get();
  }

}
