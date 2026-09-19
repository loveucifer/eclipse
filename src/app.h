#pragma once
#include "../core/window.h"
#include "../ecs/world.h"
#include "../managers/assetmanager.h"
namespace eclipse{
class App{
  public:
    App() = default;
    virtual ~App() = default;
    inline virtual core::WindowProperites GetWindowProperties(){return core::WindowProperites();}
    virtual void Initialize(ecs::World&, managers::AssetManager&){}
    virtual void OnSceneLoaded(ecs::World&, managers::AssetManager&){}
    virtual void Shutdown(){}
    virtual void Update(ecs::World&, float){}
    virtual void ImGuiRender(){}
};
}
