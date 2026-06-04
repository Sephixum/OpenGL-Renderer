#include "SystemManagerService.hpp"

namespace glr
{

  auto SystemManagerService::OnInit() -> void
  {

  }

  auto SystemManagerService::OnUpdate() -> void
  {
    for (auto& [_, sys] : m_systems)
    {
      sys->Invoke();
    }
  }

  auto SystemManagerService::OnShutdown() -> void
  {

  }

}
