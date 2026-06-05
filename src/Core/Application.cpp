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
#include "Systems/EditorUISystem.hpp"

#include "Components/Components.hpp"

#include <fstream>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/trigonometric.hpp>
#include <memory>

namespace glr
{

  auto Application::Run() -> void
  {
    Setup();
    {
      auto& window = ServiceLocator::GetInstance().Get<WindowService>();
      auto& reg    = ServiceLocator::GetInstance().Get<SceneManagerService>().GetActiveScene().registry;

      auto dummy_cam = reg.create();
      reg.emplace<Component::Camera>(dummy_cam);
      reg.emplace<Component::ActiveCamera>(dummy_cam);
      reg.emplace<Component::Transform>(dummy_cam);
      reg.emplace<Component::Projection>(dummy_cam);

      auto dummy_entity1 = reg.create();
      reg.emplace<Component::Transform>(dummy_entity1);
      auto& mesh_asset1 = reg.emplace<Component::MeshAsset>(dummy_entity1);
      reg.emplace<Component::Tag>(dummy_entity1, "dummy 1");
      mesh_asset1.tag = "FlightHelmet";

      auto dummy_entity2 = reg.create();
      auto& transform2  = reg.emplace<Component::Transform>(dummy_entity2);
      auto& mesh_asset2 = reg.emplace<Component::MeshAsset>(dummy_entity2);
      reg.emplace<Component::Tag>(dummy_entity2, "dummy 2");
      mesh_asset2.tag = "FlightHelmet";
      transform2.position = {3.0, -5.0f, -1.0f};
      transform2.rotation = 
          transform2.rotation 
        * glm::angleAxis(glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f))
        * glm::angleAxis(glm::radians(48.0f), glm::vec3(0.0f, 1.0f, 0.0f));

      // auto fox = reg.create();
      // reg.emplace<Component::Transform>(fox);
      // auto& mesh_asset3 = reg.emplace<Component::MeshAsset>(fox);
      // reg.emplace<Component::Tag>(fox, "Fox");
      // mesh_asset3.tag = "Fox";

      while (not window.ShouldClose())     
      {
        window.PollEvents();
        EditorUISystem::BeginFrame();
        ServiceLocator::GetInstance().UpdateServices();
        EditorUISystem::EndFrame();
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
    system_manager.AddSystem<EditorUISystem>();
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
