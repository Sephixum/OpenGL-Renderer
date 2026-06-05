#include "EditorUISystem.hpp"
#include "Services/TimerService.hpp"
#include "Services/ServiceLocator.hpp"
#include "Services/WindowService.hpp"
#include "Services/InputManagerService.hpp"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <ImGuizmo.h>

namespace glr
{

  auto EditorUISystem::DrawDebugInfoWindow() -> void
  {
    ImGui::Begin("Debug Info", &_state.show_debug_info);

    auto& timer = ServiceLocator::GetInstance().Get<TimerService>();
    ImGui::Text("Frame:  %lu",     timer.GetFrameCount());
    ImGui::Text("Delta:  %.3f ms", timer.GetDeltaSeconds() * 1000.0);
    ImGui::Text("Elapsed: %.1f s", std::chrono::duration<double>(timer.GetElapsedTime()).count());
    ImGui::Text("Time Scale: %.2f", timer.GetTimeScale());

    ImGui::Separator();

    ImGui::End();
  }

  EditorUISystem::EditorUISystem()
  {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    auto& window = ServiceLocator::GetInstance().Get<WindowService>();
    // Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window.GetHandle(), true);
    ImGui_ImplOpenGL3_Init("#version 460 core");

    // Optional: set up ImGuizmo
    ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
  }

  EditorUISystem::~EditorUISystem() 
  {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }

  auto EditorUISystem::Invoke() -> void
  {
    auto& io            = ImGui::GetIO();
    auto& input_manager = ServiceLocator::GetInstance().Get<InputManagerService>();

    if(io.WantCaptureKeyboard or io.WantCaptureMouse) input_manager.SetEnabled(false);
    {
      if (_state.show_debug_info) DrawDebugInfoWindow();
    }
    if((not io.WantCaptureKeyboard) and (not io.WantCaptureMouse)) input_manager.SetEnabled(true);
  }

  auto EditorUISystem::BeginFrame() -> void
  {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  auto EditorUISystem::EndFrame() -> void
  {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

}
