#pragma once

namespace glr
{

  class IService 
  {
    public:
      virtual ~IService() = default;

      virtual auto OnInit()     -> void = 0;
      virtual auto OnUpdate()   -> void = 0;
      virtual auto OnShutdown() -> void = 0;
  };

}
