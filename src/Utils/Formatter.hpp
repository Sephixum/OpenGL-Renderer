
#pragma once

#include <concepts>
#include <format>
#include <meta>
#include <string>

namespace glr
{

  template <typename T>
  concept HasToStringMember = requires(T a) 
  {
    { a.to_string() } -> std::convertible_to<std::string>;
  };
  
  template <typename T>
  concept HasToStringFree = requires(T a) 
  {
    { to_string(a) } -> std::convertible_to<std::string>;
  };

  namespace util
  {

    struct ToStringCPO
    {
      template <HasToStringMember T>
      constexpr auto operator()(T &&obj) const -> std::string
      {
        return obj.to_string();
      }

      template <typename T>
        requires(not HasToStringMember<T> and HasToStringFree<T>)
      constexpr auto operator()(T&& obj) const -> std::string
      {
        return to_string(obj);
      }
    };

    inline constexpr auto to_string = ToStringCPO{};

  }

  template <class T>
  struct Formatter
  {
    constexpr auto parse(std::format_parse_context &ctx)
    {
      return std::ranges::begin(ctx);
    }

    constexpr auto format(T const& obj, std::format_context &ctx) const
    {
      return std::format_to(ctx.out(), "{}", util::to_string(obj));
    }
  };

}

template <class T>
concept CanFormat = requires(T a) 
{
  { glr::util::to_string(a) } -> std::convertible_to<std::string>;
};

template <CanFormat T>
struct std::formatter<T> : glr::Formatter<T>
{
};
