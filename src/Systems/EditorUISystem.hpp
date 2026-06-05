#pragma once

#include "ISystem.hpp"
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
      ImGuizmo::OPERATION gizmo_op        = ImGuizmo::TRANSLATE; // W/E/R switch
      ImGuizmo::MODE      gizmo_mode      = ImGuizmo::LOCAL;        // or WORLD
      bool                gizmo_over      = false;
      bool                gizmo_using     = false;
    } _state = {};

    auto DrawDebugInfoWindow() -> void;
    auto DrawHierarchyWindow() -> void;
    auto DrawGizmo()           -> void;

    public:
      EditorUISystem(); 
      ~EditorUISystem();

      virtual auto Invoke() -> void override;

      static auto BeginFrame() -> void;
      static auto EndFrame()   -> void;
  };

}
