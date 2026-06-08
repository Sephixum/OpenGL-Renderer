#pragma once

#include "Services/IService.hpp"
#include "Utils/UniqueHandle.hpp"
#include "Utils/Utils.hpp"

#include <GLFW/glfw3.h>
#include <cstdint>

namespace glr
{

  class WindowService : public IService
  {
    UniqueHandle<::GLFWwindow*> _handle;

    public:
      virtual auto OnInit()     -> void override {}
      virtual auto OnUpdate()   -> void override {}
      virtual auto OnShutdown() -> void override {}

      WindowService(std::uint32_t width, std::uint32_t height, std::string_view title);
      WindowService() = default;

      [[nodiscard]] auto ShouldClose() -> bool;

      auto SwapBuffers() -> void;
      auto PollEvents()  -> void;

      [[nodiscard]] auto GetHandle() -> ::GLFWwindow* { return _handle; }

      [[nodiscard]] auto GetWindowWidth()  const -> u32;
      [[nodiscard]] auto GetWindowHeight() const -> u32;
  };

}
