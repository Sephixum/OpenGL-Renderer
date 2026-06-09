#include <glad.h>
#include <GLFW/glfw3.h>

#include "Utils/Log.hpp"
#include "WindowService.hpp"
#include "Core/Event.hpp"
#include "Utils/Exception.hpp"
#include "Core/Application.hpp"

namespace glr
{

  WindowService::WindowService(std::uint32_t width, std::uint32_t height, std::string_view title)
  {
    [[maybe_unused]] static auto _ = []
    {
      if (::glfwInit() == GLFW_FALSE)
      {
        throw Exception{"Failed to init GLFW"};
      }

      ::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
      ::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
      ::glfwWindowHint(GLFW_OPENGL_PROFILE       , GLFW_OPENGL_CORE_PROFILE);

      return true;
    }();


    _handle = {
      ::glfwCreateWindow(width, height, title.data(), {}, {}),
      [](::GLFWwindow* w)
      {
        ::glfwDestroyWindow(w);
      }
    };

    if (_handle.Get() == nullptr)
    {
      throw Exception{"Failed to create GLFW window"};
    }

    ::glfwMakeContextCurrent(_handle);

    if (::gladLoadGL(::glfwGetProcAddress) == 0)
    {
      throw Exception{"Failed to load glad"};
    }
    ::glViewport(0, 0, width, height);
    ::glEnable(GL_DEBUG_OUTPUT);
    ::glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    ::glDebugMessageCallback([](GLenum, GLenum, GLuint,
        GLenum severity, GLsizei, const GLchar* message, const void*)
    {
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
        std::println("[GL Debug] {}", message);
        __builtin_trap(); // ← breaks in debugger at exact call site
    }, nullptr);

    ::glfwSetWindowSizeCallback(_handle, 
    [](::GLFWwindow*, int w, int h)
    {
      Application::GetInstance().GetEventBus().trigger<Event::Resize>({w, h});
    });

    ::glfwSetScrollCallback(_handle,
    [](::GLFWwindow*, double xoffset, double yoffset)
    {
      Application::GetInstance().GetEventBus().trigger<Event::MouseScroll>({xoffset, yoffset});
    });

    ::glfwSetMouseButtonCallback(_handle,
    [](::GLFWwindow*, int button, int action, int mods)
    {
      Application::GetInstance().GetEventBus().trigger<Event::MouseButton>({button, action, mods});
    });

    ::glfwSetKeyCallback(_handle,
    [](::GLFWwindow*, int key, int scancode, int action, int mods)
    {
      Application::GetInstance().GetEventBus().trigger<Event::KeyBoardKey>({key, scancode, action, mods});
    });

    ::glfwSetCursorPosCallback(_handle,
    [](::GLFWwindow*, double xpos, double ypos)
    {
      Application::GetInstance().GetEventBus().trigger<Event::MouseMove>({xpos, ypos});
    });
  }

  auto WindowService::ShouldClose() -> bool
  {
    return ::glfwWindowShouldClose(_handle);
  }

  auto WindowService::SwapBuffers() -> void
  {
    ::glfwSwapBuffers(_handle);
  }

  auto WindowService::PollEvents() -> void
  {
    ::glfwPollEvents();
  }



  auto WindowService::GetWidth()  const -> u32
  {
    auto w = i32{};
    glfwGetWindowSize(_handle, &w, nullptr);
    return w;
  }

  auto WindowService::GetHeight() const -> u32
  {
    auto h = i32{};
    glfwGetWindowSize(_handle, nullptr, &h);
    return h;
  }

}
