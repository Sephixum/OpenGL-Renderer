#pragma once

namespace glr::Event
{

  struct Resize
  {
    int width;
    int height;
  };

  struct MouseButton
  {
    int button;   
    int action;   
    int mods;   
  };

  struct MouseScroll
  {
    double xoffset;
    double yoffset;
  };

  struct MouseMove
  {
    double xpos;
    double ypos;
  };

  struct KeyBoardKey
  {
    int key;
    int scancode;
    int action;
    int mods;
  };

  struct CameraSwitch
  {

  };

}
