#include "EditorUISystem.hpp"

#include "Components/include/MeshAssetComponent.hpp"
#include "Components/include/ProjectionComponent.hpp"
#include "Services/SceneManagerService.hpp"
#include "Services/TimerService.hpp"
#include "Services/ResourceLoaderService.hpp"
#include "Services/ServiceLocator.hpp"
#include "Services/MeshManagerService.hpp"
#include "Services/WindowService.hpp"
#include "Services/InputManagerService.hpp"

#include "Components/Components.hpp"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <ImGuizmo.h>
#include <ImGuiFileDialog.h>

namespace glr
{

  auto EditorUISystem::DrawFileDialogs() -> void
  {
    if (_file_dialog.Display("ChooseModelDlg"))
    {
      if (_file_dialog.IsOk())
      {
        auto file_path = _file_dialog.GetFilePathName();
        ServiceLocator::GetInstance()
          .Get<ResourceLoaderService>()
          .LoadModel(file_path);
      }
      
      _file_dialog.Close();
    }
  }

  auto EditorUISystem::DrawMainMenuBar() -> void
  {
    if (ImGui::BeginMainMenuBar())
    {
      if (ImGui::BeginMenu("File"))
      {
        if (ImGui::MenuItem("Load Model...", "Ctrl+L"))
        {
          _file_dialog.OpenDialog("ChooseModelDlg", "Choose Model", "3D Models (*.gltf *.glb *.obj){.gltf,.glb,.obj}");
        }
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }
  }

  auto EditorUISystem::DrawEntityInspector() -> void
  {

    auto& scene   = ServiceLocator::GetInstance().Get<SceneManagerService>().GetActiveScene();
    auto& reg     = scene.registry;

    if (_state.selected_entity == entt::null or (not reg.valid(_state.selected_entity)))
    {
      return;
    }
    ImGui::Begin("Inspector");

    auto entity = _state.selected_entity;

    auto entityLabel = std::format("Entity {}", static_cast<std::size_t>(entity));
    if (auto* tag = reg.try_get<Component::Tag>(entity))
    {
      entityLabel = tag->str;
    }
    ImGui::Text("Selected: %s", entityLabel.c_str());
    ImGui::Separator();


    // ---- Transform ----
    if (auto* t = reg.try_get<Component::Transform>(entity))
    {
      if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
      {
        glm::vec3 pos = t->position;
        if (ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.1f))
        {
          t->position = pos;
        }
        glm::vec3 euler = glm::degrees(glm::eulerAngles(t->rotation));
        if (ImGui::DragFloat3("Rotation", glm::value_ptr(euler), 1.0f))
        {
            t->rotation = glm::quat(glm::radians(euler));
        }

        glm::vec3 scale = t->scale;
        if (ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.1f, 0.01f, 100.0f))
        {
            t->scale = scale;
        }
      }
    }

    if (auto* mesh = reg.try_get<Component::MeshAsset>(entity))
    {
      if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
      {
        auto& meshManager = ServiceLocator::GetInstance().Get<MeshManagerService>();
        auto meshNames    = meshManager.GetModelNames();

        auto currentMesh = mesh->mesh_tag;
        if (ImGui::BeginCombo("Model", currentMesh.c_str()))
        {
          for (const auto& name : meshNames)
          {
            auto isSelected = (currentMesh == name);
            if (ImGui::Selectable(name.data(), isSelected))
            {
              mesh->mesh_tag = name;
            }   
            if (isSelected)
            {
              ImGui::SetItemDefaultFocus();
            }
          }
          ImGui::EndCombo();
        }
      }
    }
    else
    {
      if (ImGui::Button("Add Mesh Component"))
      {
        reg.emplace<Component::MeshAsset>(entity);
      }
    }

    // ---- Tag ----
    if (auto* tag = reg.try_get<Component::Tag>(entity))
    {
      if (ImGui::CollapsingHeader("Tag"))
      {
        auto buf = tag->str;
        buf.resize(512);
        if (ImGui::InputText("Name", buf.data(), buf.size()))
        {
          buf.resize(std::strlen(buf.c_str()));
          tag->str = std::move(buf);
        }
      }
    }

    ImGui::End();
  }

  auto EditorUISystem::DrawLoadedModels() -> void
  {
    auto const names = ServiceLocator::GetInstance().Get<MeshManagerService>().GetModelNames();

    ImGui::Begin("Mesh names", &_state.show_debug_info);
    {
      if (names.empty())
      {
        ImGui::Text("No models loaded");
      }
      else
      {
        for (auto const& name : names)
        {
          ImGui::Text("%s", name.data());
        }
      }
    }
    ImGui::End();
  }

  auto EditorUISystem::DrawDebugInfoWindow() -> void
  {
    ImGui::Begin("Debug Info", &_state.show_debug_info);

    auto& timer = ServiceLocator::GetInstance().Get<TimerService>();
    auto  fps   = (timer.GetDeltaSeconds() > 0.0) ? (1.0 / timer.GetDeltaSeconds()) : 0.0;
    ImGui::Text("FPS       : %.3f", fps);
    ImGui::Text("Frame     : %lu",     timer.GetFrameCount());
    ImGui::Text("Delta     : %.3f ms", timer.GetDeltaSeconds() * 1000.0);
    ImGui::Text("Elapsed   : %.1f s", std::chrono::duration<double>(timer.GetElapsedTime()).count());
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
        ::ImGuizmo::TRANSLATE | ImGuizmo::SCALE | ImGuizmo::ROTATE,
        ::ImGuizmo::LOCAL,
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
      auto final_tag = tag.str;
      if (final_tag.empty()) final_tag = "Uknown Entity";
      if (ImGui::Selectable(final_tag.c_str(), _state.selected_entity == entity)) 
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
    : _file_dialog{}
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
      DrawEntityInspector();
      DrawMainMenuBar();
      DrawFileDialogs();
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
