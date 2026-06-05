#pragma once

#include "ISystem.hpp"

namespace glr
{

  class EditorUISystem : public ISystem
  {
    struct State
    {
      bool show_debug_info = true;
    } _state = {};

    auto DrawDebugInfoWindow() -> void;
    public:
      EditorUISystem(); 
      ~EditorUISystem();

      virtual auto Invoke() -> void override;

      static auto BeginFrame() -> void;
      static auto EndFrame()   -> void;
  };

}
