
#pragma once

#include <format>
#include <stacktrace>
#include <stdexcept>
#include <string>

namespace glr
{

  class Exception : public std::runtime_error
  {
    public:
      template <class... Args>
      constexpr Exception(std::format_string<Args...> msg, Args &&...args);

      constexpr auto to_string(this auto &&self) -> std::string;

      constexpr auto what() const noexcept -> char const* override;

    private:
      std::string _what;
  };
  
  template <class... Args>
  constexpr Exception::Exception(std::format_string<Args...> msg, Args &&...args)
      : std::runtime_error{std::format(msg, std::forward<Args>(args)...)}
      , _what{std::format("{}\n{}", std::runtime_error::what(), std::stacktrace::current(1)).c_str()}
  {
  }
  
  constexpr auto Exception::to_string(this auto &&self) -> std::string
  {
    return self._what;
  }
  
  constexpr auto Exception::what() const noexcept -> const char *
  {
    return _what.c_str();
  }

}
