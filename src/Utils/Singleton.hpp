#pragma once

namespace glr
{

  template <typename T>
  struct Singleton
  {
    Singleton()          = default;
    virtual ~Singleton() = default;
    
    static auto& GetInstance()
    {
      static auto instance = T{};
      return instance;
    }

    Singleton(Singleton&&)                         = delete;
    auto operator=(Singleton&&)      -> Singleton& = delete;
    Singleton(Singleton const&)                    = delete;
    auto operator=(Singleton const&) -> Singleton& = delete;
  };

}
