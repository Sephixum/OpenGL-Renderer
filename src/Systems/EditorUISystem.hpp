#pragma once

#include "ISystem.hpp"
#include "ImGuiFileDialog.h"
#include <entt/entt.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

namespace glr
{

  class EditorUISystem : public ISystem
  {
    struct State
    {
      bool show_debug_info = true;

      // Gizmo
      entt::entity        selected_entity = entt::null;
    } _state = {};

    IGFD::FileDialog _file_dialog = {};

    auto DrawDebugInfoWindow() -> void;
    auto DrawHierarchyWindow() -> void;
    auto DrawGizmo() -> void;
    auto DrawLoadedModels() -> void;
    auto DrawEntityInspector() -> void;
    auto DrawMainMenuBar()    -> void;
    auto DrawFileDialogs()    -> void;
    auto DrawTextureViewer()  -> void;

    public:
      EditorUISystem(); 
      ~EditorUISystem();

      virtual auto Invoke() -> void override;

      static auto BeginFrame() -> void;
      static auto EndFrame()   -> void;
  };

}
