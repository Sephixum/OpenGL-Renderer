#pragma once

#include <filesystem>
#include <format>
#include <fstream>
#include <mutex>
#include <print>
#include <source_location>
#include <string>
#include <string_view>
#include <vector>

// #include "../fps.concurrency/lock.hpp"
// #include "config.hpp"

namespace glr::log
{

  inline std::vector<std::string> history{};
  // inline Lock history_lock{};

  namespace impl
  {

    inline static auto log_file = []
    {
      auto f = std::ofstream{"log", std::ios::app};
      if (!f)
      {
        std::terminate();
      }
      return f;
    }();

  }

  enum class Level
  {
    DEBUG,
    INFO,
    WARN,
    ERR
  };

  inline auto g_force_log = false;

  template <Level L, class... Args>
  struct Print
  {
    Print(std::format_string<Args...> msg, Args &&...args, std::source_location loc = std::source_location::current())
    {
      auto c = '?';
      if constexpr (L == Level::DEBUG)
      {
        c = 'D';
      }
      else if constexpr (L == Level::INFO)
      {
        c = 'I';
      }
      else if constexpr (L == Level::WARN)
      {
        c = 'W';
      }
      else if constexpr (L == Level::ERR)
      {
        c = 'E';
      }

      const auto path = std::filesystem::path{loc.file_name()};

      auto log_line = std::format(
        "[{}] {}:{} {}", c, path.filename().string(), loc.line(), std::format(msg, std::forward<Args>(args)...));

      {
        // const auto lock = std::scoped_lock(history_lock);

        std::println("{}", log_line);

        if constexpr (true)
        {
            impl::log_file << log_line << std::endl;
        }

        history.push_back(std::move(log_line));
      }
    }
  };

template <Level L = {}, class... Args>
Print(std::format_string<Args...>, Args &&...) -> Print<L, Args...>;

template <class... Args>
using Debug = Print<Level::DEBUG, Args...>;

template <class... Args>
using Info = Print<Level::INFO, Args...>;

template <class... Args>
using Warn = Print<Level::WARN, Args...>;

template <class... Args>
using Error = Print<Level::ERR, Args...>;

}
