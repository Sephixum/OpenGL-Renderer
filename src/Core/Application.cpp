#include "Application.hpp"

#include "Components/include/CameraComponent.hpp"
#include "Components/include/MeshAssetComponent.hpp"
#include "Components/include/ProjectionComponent.hpp"
#include "Events/EventBus.hpp"

#include "Graphics/Buffer.hpp"
#include "Graphics/GraphicsPipeline.hpp"
#include "Graphics/VertexArray.hpp"

#include "Services/MeshManagerService.hpp"
#include "Services/InputManagerService.hpp"
#include "Services/ShaderManagerService.hpp"
#include "Services/WindowService.hpp"
#include "Services/ResourceLoaderService.hpp"
#include "Services/ServiceLocator.hpp"
#include "Services/SystemManagerService.hpp"
#include "Services/TimerService.hpp"
#include "Services/SceneManagerService.hpp"

#include "Systems/CameraSystem.hpp"
#include "Systems/FlyCameraControllerSystem.hpp"
#include "Systems/RenderingSystem.hpp"

#include "Components/Components.hpp"

#include <fstream>
#include <memory>

namespace glr
{

  auto Application::Run() -> void
  {
    Setup();
    {
      auto& window = ServiceLocator::GetInstance().Get<WindowService>();
      auto& reg    = ServiceLocator::GetInstance().Get<SceneManagerService>().GetActiveScene().scene;

      auto dummy_cam = reg.create();
      reg.emplace<Component::Camera>(dummy_cam);
      reg.emplace<Component::ActiveCamera>(dummy_cam);
      reg.emplace<Component::Transform>(dummy_cam);
      reg.emplace<Component::Projection>(dummy_cam);

      auto dummy_entity = reg.create();
      reg.emplace<Component::Transform>(dummy_entity);
      auto& mesh_asset = reg.emplace<Component::MeshAsset>(dummy_entity);
      mesh_asset.tag = "FlightHelmet";

      while (not window.ShouldClose())     
      {
        window.PollEvents();
        ServiceLocator::GetInstance().UpdateServices();
        window.SwapBuffers();
      }
    }
    CleanUp();
  }

  auto Application::Setup() -> void
  {
    ServiceLocator::GetInstance().Emplace<TimerService>();
    ServiceLocator::GetInstance().Emplace<WindowService>(800, 600, "GLR");
    ServiceLocator::GetInstance().Emplace<MeshManagerService>();
    ServiceLocator::GetInstance().Emplace<ShaderManagerService>();
    ServiceLocator::GetInstance().Emplace<ResourceLoaderService>();
    ServiceLocator::GetInstance().Emplace<InputManagerService>();
    ServiceLocator::GetInstance().Emplace<SystemManagerService>();
    ServiceLocator::GetInstance().Emplace<SceneManagerService>();

    ServiceLocator::GetInstance().InitServices();

    auto& system_manager = ServiceLocator::GetInstance().Get<SystemManagerService>();
    system_manager.AddSystem<CameraSystem>();
    system_manager.AddSystem<FlyCameraControllerSystem>();
    system_manager.AddSystem<RenderingSystem>();
  }

  auto Application::CleanUp() -> void
  {
    ServiceLocator::GetInstance().ShutdownServices();
  }

  auto Application::GetEventBus() -> EventBus&
  {
    return _event_bus;
  }

}
