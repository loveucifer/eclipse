#pragma once
#include "../core/window.h"
#include "../managers/logmanager.h"
#include "../managers/rendermanager.h"
#include "app.h"


namespace eclipse {
class App;
class engine {
public:
  static engine &Instance();
  ~engine();
  void Run(App* app);
  void Quit();
  // [[nodiscard]]bool Initialize(); // we have to use the return for something
  // so we use it for our while loop
  //  void Shutdown();

  // managers
  inline managers::RenderManager &GetRenderManager() { return mRenderManager; }
  inline App& GetApp(){return* mApp;}
  inline core::window &GetWindow() { return mWindow; }
  // if we call rendermanager we get referncce to it

private:
  core::window mWindow;

  [[nodiscard]] bool Initialize(); // we have to use the return for something so
                                   // we use it for our while loop
  void Shutdown();
  void Update();
  void Render();
  engine();

  static engine *mInstance;
  App* mApp;
  // managers
  managers::LogManager mLogManager;
  managers::RenderManager mRenderManager;

  bool mIsRunning;

  bool mIsInitialized;

  void getInfo();
};

} // namespace eclipse
