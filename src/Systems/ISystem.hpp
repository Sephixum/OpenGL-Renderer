#pragma once

namespace glr
{

  struct ISystem 
  {
    virtual      ~ISystem()       = default;
    virtual auto Invoke() -> void = 0;
  };

}
