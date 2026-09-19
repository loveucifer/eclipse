#include "engine.h"

#include "../input/keyboard.h"
#include "../input/mouse.h"
#include "../input/actions.h"
#include "../src/log.h"

#include <SDL2/SDL.h>

#include <cstdlib>
#include <filesystem>
#include <string>

namespace {

std::filesystem::path FindAssetRoot() {
  if (const char* overrideRoot = std::getenv("ECLIPSE_ASSET_ROOT")) {
    return overrideRoot;
  }

  char* basePath = SDL_GetBasePath();
  if (basePath) {
    const std::filesystem::path packaged =
        std::filesystem::path(basePath) / "assets";
    SDL_free(basePath);
    if (std::filesystem::is_regular_file(packaged / "manifest.json")) {
      return packaged;
    }
  }
  return "assets";
}

} // namespace

namespace eclipse {

Engine::Engine() : mIsInitialized(false), mIsRunning(false), mApp(nullptr) {}

Engine::~Engine() {
  if (mIsInitialized) {
    Shutdown();
  }
}

void Engine::Run(App* app) {
  mLogManager.Initialize();
  ECLIPSE_ASSERT(app != nullptr, "cannot run a null app");
  ECLIPSE_ASSERT(mApp == nullptr, "trying to reinitialize app");
  if (!app || mApp) {
    mLogManager.Shutdown();
    return;
  }

  mApp = app;
  if (!Initialize()) {
    mApp = nullptr;
    mLogManager.Shutdown();
    return;
  }

  while (mIsRunning) {
    Update();
    if (mIsRunning) {
      Render();
    }
  }
  Shutdown();
}

void Engine::Update() {
  mWindow.PollEvents();
  mClock.Tick();
  if (!mIsRunning) {
    return;
  }

  if (input::keyboard::KeyDown(input::ECLIPSE_INPUT_KEY_F5)) {
    ReloadScene();
    return;
  }
  if (mGameStateManager.Current() != managers::GameState::Playing) {
    return;
  }

  const float deltaTime = mClock.GetDeltaTime();
  mMovementSystem.Update(mWorld, deltaTime);
  mNpcSystem.Update(mWorld, deltaTime);
  mCollisionSystem.Update(mWorld);
  mAnimationSystem.Update(mWorld, deltaTime);
  mApp->Update(mWorld, deltaTime);
  mPhysicsSystem.Update(mWorld, deltaTime);
  mCameraSystem.Update(mWorld, mWindow.GetSize());
  mWorld.FlushDestroyed();
}

void Engine::ReloadScene() {
  if (std::filesystem::path(mSceneSource).extension() == ".json") LoadSceneFile(mSceneSource);
  else LoadScene(mSceneSource);
}

bool Engine::LoadScene(const std::string& name) {
  const auto source = name;
  if (!mSceneManager.Load(source, mWorld, mAssetManager)) return false;
  mSceneSource = source; SceneLoaded(); return true;
}

bool Engine::SaveScene(const std::filesystem::path& path) {
  return mSceneManager.SaveFile(path, mWorld, mAssetManager);
}

bool Engine::LoadSceneFile(const std::filesystem::path& path) {
  if (!mSceneManager.LoadFile(path, mWorld, mAssetManager)) return false;
  mSceneSource = path.string(); SceneLoaded(); return true;
}

void Engine::SceneLoaded() {
  mPhysicsSystem.Reset();
  if (mApp) mApp->OnSceneLoaded(mWorld, mAssetManager);
  mCameraSystem.Update(mWorld, mWindow.GetSize());
}

void Engine::Render() {
  mWindow.BeginRender();
  mRenderSystem.Render(mWorld, mRenderManager);
  mWindow.EndRender();
}

void Engine::Quit() { mIsRunning = false; }

Engine& Engine::Instance() {
  static Engine instance;
  return instance;
}

bool Engine::Initialize() {
  ECLIPSE_ASSERT(!mIsInitialized, "trying to reinitialize engine");
  GetInfo();

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    ECLIPSE_ERROR("Error initializing SDL2: {}", SDL_GetError());
    return false;
  }

  SDL_version version;
  SDL_VERSION(&version);
  ECLIPSE_INFO("SDL {}.{}.{}", static_cast<int>(version.major),
               static_cast<int>(version.minor),
               static_cast<int>(version.patch));

  const core::WindowProperites properties = mApp->GetWindowProperties();
  if (!mWindow.Create(properties)) {
    SDL_Quit();
    return false;
  }

  mRenderManager.Initialize();
  mClock.Initialize();
  if (!mAudioManager.Initialize()) {
    ECLIPSE_WARN("Continuing without audio output");
  }
  if (!mAssetManager.Initialize(FindAssetRoot())) {
    mAudioManager.Shutdown();
    mRenderManager.Shutdown();
    mWindow.Shutdown();
    SDL_Quit();
    return false;
  }

  input::mouse::Initialize();
  input::keyboard::Initialize();
  input::InitializeActions();
  mSceneManager.Register(
      "main", [this](ecs::World& world, managers::AssetManager& assets) {
        mApp->Initialize(world, assets);
      });

  mIsInitialized = true;
  mIsRunning = true;
  if (!LoadScene("main")) {
    Shutdown();
    return false;
  }
  mGameStateManager.Set(managers::GameState::Playing);
  return true;
}

void Engine::Shutdown() {
  mIsRunning = false;
  if (mApp) {
    mApp->Shutdown();
  }

  mWorld.Clear();
  mAssetManager.Clear();
  mRenderManager.Shutdown();
  mAudioManager.Shutdown();
  mWindow.Shutdown();
  SDL_Quit();

  mIsInitialized = false;
  mApp = nullptr;
  mLogManager.Shutdown();
}

void Engine::GetInfo() {
#ifdef ECLIPSE_CONFIG_DEBUG
  ECLIPSE_DEBUG("Configuration: DEBUG");
#endif
#ifdef ECLIPSE_CONFIG_RELEASE
  ECLIPSE_DEBUG("Configuration: RELEASE");
#endif
#ifdef ECLIPSE_PLATFORM_MAC
  ECLIPSE_WARN("Platform: MAC");
#endif
#ifdef ECLIPSE_PLATFORM_WINDOWS
  ECLIPSE_WARN("Platform: WINDOWS");
#endif
#ifdef ECLIPSE_PLATFORM_LINUX
  ECLIPSE_WARN("Platform: LINUX");
#endif
}

} // namespace eclipse
