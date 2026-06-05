#include "EditorUISystem.hpp"

#include "Components/include/ProjectionComponent.hpp"
#include "Services/SceneManagerService.hpp"
#include "Services/TimerService.hpp"
#include "Services/ServiceLocator.hpp"
#include "Services/WindowService.hpp"
#include "Services/InputManagerService.hpp"

#include "Components/Components.hpp"

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

    ImGui::End();
  }

  auto EditorUISystem::DrawGizmo() -> void
  {
    auto& scene   = ServiceLocator::GetInstance().Get<SceneManagerService>().GetActiveScene();
    auto& reg     = scene.registry;
    auto& io      = ImGui::GetIO();

    if (_state.selected_entity == entt::null or (not reg.valid(_state.selected_entity)))
    {
      return;
    }

    auto* transform = reg.try_get<Component::Transform>(_state.selected_entity);
    if (!transform) return;

    glm::mat4 model = transform->GetMatrix();

    ImGuizmo::BeginFrame();
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::Enable(true);
    ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    
    auto cam = scene.GetActiveCamera();
    
    glm::mat4 view_mat = [&]
    {
      return reg.get<Component::Transform>(cam).GetView();
    }();
    glm::mat4 proj_mat  = [&]
    {
      return reg.get<Component::Projection>(cam).GetMatrix();
    }();
    
    ImGuizmo::Manipulate(
        glm::value_ptr(view_mat),
        glm::value_ptr(proj_mat),
        ::ImGuizmo::UNIVERSAL,
        ::ImGuizmo::WORLD,
        glm::value_ptr(model)
    );

    if (ImGuizmo::IsUsing())
    {
      glm::vec3 new_pos;
      glm::vec3 new_scale;
      glm::vec3 new_rotation;
      ImGuizmo::DecomposeMatrixToComponents(
          glm::value_ptr(model),
          glm::value_ptr(new_pos),
          glm::value_ptr(new_rotation),
          glm::value_ptr(new_scale)
      );
      transform->position = new_pos;
      transform->rotation = glm::qua(glm::radians(new_rotation));
      transform->scale    = new_scale;
    }
  }

  auto EditorUISystem::DrawHierarchyWindow() -> void
  {
    ImGui::Begin("Hierarchy");

    auto& reg  = ServiceLocator::GetInstance().Get<SceneManagerService>().GetActiveScene().registry;
    auto  view = reg.view<Component::Transform, Component::Tag>();

    for (auto [entity, transform, tag] : view.each()) 
    {
      if (ImGui::Selectable(tag.str.c_str(), _state.selected_entity == entity)) 
      {
        _state.selected_entity = entity;
      }
    }

    if (ImGui::IsWindowHovered() and ImGui::IsMouseClicked(0))
    {
      _state.selected_entity = entt::null;
    }

    ImGui::End();
  }

  EditorUISystem::EditorUISystem()
  {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    auto& window = ServiceLocator::GetInstance().Get<WindowService>();
    ImGui_ImplGlfw_InitForOpenGL(window.GetHandle(), true);
    ImGui_ImplOpenGL3_Init("#version 460 core");

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
      DrawHierarchyWindow();
      DrawGizmo();
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
