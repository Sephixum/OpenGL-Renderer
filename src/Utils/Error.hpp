#pragma once

#include <format>
#include <memory>
#include <stacktrace>
#include <utility>

#include "Exception.hpp"
#include "Log.hpp"
#include "UniqueHandle.hpp"

namespace glr
{

  template <class... Args>
  constexpr auto Expect(bool predicate, std::format_string<Args...> msg, Args &&...args) -> void;
  
  template <class T, class... Args>
  constexpr auto Expect(std::unique_ptr<T> &obj, std::format_string<Args...>(msg), Args &&...args) -> void;
  
  template <class... Args>
  constexpr auto Ensure(bool predicate, std::format_string<Args...> msg, Args &&...args) -> void;
  
  template <class T, T Invalid, class... Args>
  constexpr auto Ensure(UniqueHandle<T, Invalid> &obj, std::format_string<Args...> msg, Args &&...args) -> void;
  
  template <class T, class D, class... Args>
  constexpr auto Ensure(std::unique_ptr<T, D> &obj, std::format_string<Args...> msg, Args &&...args) -> void;
  
  template <class... Args>
  constexpr auto Expect(bool predicate, std::format_string<Args...> msg, Args &&...args) -> void
  {
    if (!predicate)
    {
      log::Error("{}", std::format(msg, std::forward<Args>(args)...));
      log::Error("{}", std::stacktrace::current(1));
      std::terminate();
      std::unreachable();
    }
  }
  
  template <class T, class... Args>
  constexpr auto Expect(std::unique_ptr<T> &obj, std::format_string<Args...>(msg), Args &&...args) -> void
  {
    Expect(!!obj, msg, std::forward<Args>(args)...);
  }
  
  template <class... Args>
  constexpr auto Ensure(bool predicate, std::format_string<Args...> msg, Args &&...args) -> void
  {
    if (!predicate)
    {
      throw Exception(msg, std::forward<Args>(args)...);
    }
  }
  
  template <class T, T Invalid, class... Args>
  constexpr auto Ensure(UniqueHandle<T, Invalid> &obj, std::format_string<Args...> msg, Args &&...args) -> void
  {
    Ensure(!!obj, msg, std::forward<Args>(args)...);
  }
  
  template <class T, class D, class... Args>
  constexpr auto Ensure(std::unique_ptr<T, D> &obj, std::format_string<Args...> msg, Args &&...args) -> void
  {
    Ensure(!!obj, msg, std::forward<Args>(args)...);
  }

}
