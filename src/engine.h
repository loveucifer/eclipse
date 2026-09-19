#pragma once

// LEARNING MAP
// Engine owns the long-lived services and runs the game loop. Study Run(),
// Update(), and Render() in engine.cpp. The loop order is important: input,
// gameplay systems, app logic, camera, then drawing.

#include "../core/window.h"
#include "../managers/rendermanager.h"
#include "../managers/logmanager.h"
#include "../managers/assetmanager.h"
#include "../managers/audiomanager.h"
#include "../managers/gamestatemanager.h"
#include "../managers/scenemanager.h"
#include "app.h"
#include "../systems/render_system.h"
#include "../systems/movement_system.h"
#include "../systems/camera_system.h"
#include "../systems/animation_system.h"
#include "../systems/collision_system.h"
#include "../systems/npc_system.h"
#include "../systems/physics_system.h"
#include "../core/clock.h"
#include <filesystem>
namespace eclipse{
  class Engine{
    public:
      void Run(App* app);
      void Quit();
      void ReloadScene();
      bool LoadScene(const std::string& name);
      bool SaveScene(const std::filesystem::path& path);
      bool LoadSceneFile(const std::filesystem::path& path);
      const std::string& GetSceneSource() const { return mSceneSource; }
      static Engine& Instance();
      inline core::Window& GetWindow(){return mWindow;}
      inline App& GetApp(){return* mApp;}
      inline managers::RenderManager& GetRenderManager(){return mRenderManager;}
      inline ecs::World& GetWorld(){return mWorld;}
      inline managers::AssetManager& GetAssetManager(){return mAssetManager;}
      inline managers::AudioManager& GetAudioManager(){return mAudioManager;}
      inline managers::GameStateManager& GetGameState(){return mGameStateManager;}
      inline managers::SceneManager& GetSceneManager(){return mSceneManager;}
      inline systems::PhysicsSystem& GetPhysicsSystem(){return mPhysicsSystem;}
    private:

      void Update();
      void Render();
      bool mIsInitialized;
      bool mIsRunning;
      managers::RenderManager mRenderManager;
      ecs::World mWorld;
      systems::RenderSystem mRenderSystem;
      systems::MovementSystem mMovementSystem;
      systems::CameraSystem mCameraSystem;
      systems::AnimationSystem mAnimationSystem;
      systems::CollisionSystem mCollisionSystem;
      systems::NpcSystem mNpcSystem;
      systems::PhysicsSystem mPhysicsSystem;
      core::Clock mClock;
      core::Window mWindow;
      App* mApp;
      managers::LogManager mLogManager;
      managers::AssetManager mAssetManager;
      managers::AudioManager mAudioManager;
      managers::GameStateManager mGameStateManager;
      managers::SceneManager mSceneManager;
      std::string mSceneSource = "main";
      void SceneLoaded();
      Engine();
      ~Engine();
      bool Initialize();
      void Shutdown();
      void GetInfo();

  };
}
