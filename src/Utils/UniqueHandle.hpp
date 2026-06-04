#pragma once

#include <functional>
#include <ranges>
#include <utility>

namespace glr
{

  template <class T, T Invalid = {}>
      requires std::is_trivially_copyable_v<T>
  class UniqueHandle
  {
    T                      _obj;
    std::function<void(T)> _deleter;

    public:
      constexpr UniqueHandle();

      constexpr UniqueHandle(T obj, std::function<void(T)> deleter);

      constexpr ~UniqueHandle();
      UniqueHandle(const UniqueHandle&) = delete;

      auto operator=(const UniqueHandle&) -> UniqueHandle& = delete;

      constexpr UniqueHandle(UniqueHandle&& other);

      constexpr auto operator=(UniqueHandle&& other) -> UniqueHandle&;

      constexpr auto Swap(UniqueHandle& other) noexcept -> void;

      constexpr auto Reset(T obj) -> void;

      constexpr auto Release() -> T;

      constexpr T Get() const;

      constexpr operator T() const;

      constexpr explicit operator bool() const;

      constexpr T* operator&() noexcept;
  };
  
  template <class T, T Invalid>
      requires std::is_trivially_copyable_v<T>
  constexpr UniqueHandle<T, Invalid>::UniqueHandle()
      : UniqueHandle(Invalid, nullptr)
  {
  }
  
  template <class T, T Invalid>
      requires std::is_trivially_copyable_v<T>
  constexpr UniqueHandle<T, Invalid>::UniqueHandle(T obj, std::function<void(T)> deleter)
      : _obj(obj)
      , _deleter(deleter)
  {
  }
  
  template <class T, T Invalid>
      requires std::is_trivially_copyable_v<T>
  constexpr UniqueHandle<T, Invalid>::~UniqueHandle()
  {
      if ((_obj != Invalid) && _deleter)
      {
          _deleter(_obj);
      }
  }
  
  template <class T, T Invalid>
      requires std::is_trivially_copyable_v<T>
  constexpr UniqueHandle<T, Invalid>::UniqueHandle(UniqueHandle &&other)
      : UniqueHandle()
  {
      Swap(other);
  }
  
  template <class T, T Invalid>
      requires std::is_trivially_copyable_v<T>
  constexpr auto UniqueHandle<T, Invalid>::operator=(UniqueHandle &&other) -> UniqueHandle &
  {
      UniqueHandle new_obj{std::move(other)};
      Swap(new_obj);
  
      return *this;
  }
  
  template <class T, T Invalid>
      requires std::is_trivially_copyable_v<T>
  constexpr auto UniqueHandle<T, Invalid>::Swap(UniqueHandle &other) noexcept -> void
  {
      std::ranges::swap(_obj, other._obj);
      std::ranges::swap(_deleter, other._deleter);
  }
  
  template <class T, T Invalid>
      requires std::is_trivially_copyable_v<T>
  constexpr auto UniqueHandle<T, Invalid>::Reset(T obj) -> void
  {
      if ((_obj != Invalid) && _deleter)
      {
          _deleter(_obj);
      }
  
      _obj = obj;
  }
  
  template <class T, T Invalid>
      requires std::is_trivially_copyable_v<T>
  constexpr T UniqueHandle<T, Invalid>::Get() const
  {
      return _obj;
  }
  
  template <class T, T Invalid>
      requires std::is_trivially_copyable_v<T>
  constexpr UniqueHandle<T, Invalid>::operator T() const
  {
      return _obj;
  }
  
  template <class T, T Invalid>
      requires std::is_trivially_copyable_v<T>
  constexpr UniqueHandle<T, Invalid>::operator bool() const
  {
      return _obj != Invalid;
  }
  
  template <class T, T Invalid>
      requires std::is_trivially_copyable_v<T>
  constexpr T *UniqueHandle<T, Invalid>::operator&() noexcept
  {
      return std::addressof(_obj);
  }

  template <typename T, T Invalid>
      requires std::is_trivially_copyable_v<T>
  constexpr auto UniqueHandle<T, Invalid>::Release() -> T
  {
    auto temp = _obj;
    _obj      = Invalid;

    return temp;
  }

}
