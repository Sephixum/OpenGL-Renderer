#pragma once

#include "IService.hpp"
#include "Utils/Singleton.hpp"

#include <concepts>
#include <flat_map>
#include <format>
#include <memory>
#include <stdexcept>
#include <typeindex>

namespace glr 
{
  
  class ServiceLocator : public Singleton<ServiceLocator>
  {
    std::flat_map<std::type_index, std::unique_ptr<IService>> _services;

    public:
      auto InitServices()     -> void;
      auto UpdateServices()   -> void;
      auto ShutdownServices() -> void;

      template<std::derived_from<IService> T, typename... Args>
        requires std::constructible_from<T, Args...>
      auto Emplace(Args&&... args) -> void;

      template <std::derived_from<IService> T>
      [[nodiscard]] auto Get() -> T&;

      template <std::derived_from<IService> T>
      [[nodiscard]] auto TryGet() -> T*;
  };
  
  template<std::derived_from<IService> T, typename... Args>
    requires std::constructible_from<T, Args...>
  auto ServiceLocator::Emplace(Args&&... args) -> void 
  {
    static std::type_index const s_type_index = typeid(T);
    if (_services.find(s_type_index) != _services.end())
    {
      throw std::invalid_argument{std::format("Service with mangled name {} is already registered...", s_type_index.name())};
    }

    _services[s_type_index] = std::make_unique<T>(std::forward<Args>(args)...);
  }
  
  template <std::derived_from<IService> T>
  auto ServiceLocator::Get() -> T&
  {
    static std::type_index const s_type_index = typeid(T);
    auto service_iterator = _services.find(s_type_index);
    if (service_iterator == _services.end())
    {
      throw std::invalid_argument{std::format("Service with mangled name {} is not registered...", s_type_index.name())};
    }

    return static_cast<T&>(*service_iterator->second);
  }
  
  template <std::derived_from<IService> T>
  auto ServiceLocator::TryGet() -> T*
  {
    static std::type_index const s_type_index = typeid(T);
    auto service_iterator = _services.find(s_type_index);
    if (service_iterator == _services.end())
    {
      throw std::invalid_argument{std::format("Service with mangled name {} is not registered...", s_type_index.name())};
    }

    return static_cast<T*>(service_iterator->second.get());
  }

  inline auto ServiceLocator::InitServices() -> void
  {
    for (auto [_, s] : _services)
    {
      s->OnInit();
    }
  }

  inline auto ServiceLocator::UpdateServices() -> void
  {
    for (auto [_, s] : _services)
    {
      s->OnUpdate();
    }
  }

  inline auto ServiceLocator::ShutdownServices() -> void
  {
    for (auto [_, s] : _services)
    {
      s->OnShutdown();
    }
  }

}
