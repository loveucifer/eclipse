#include "engine.h"
#include "../input/keyboard.h"
#include "../input/mouse.h"
#include "../managers/rendermanager.h"
#include "SDL2/SDL_error.h"
#include "app.h"
#include "log.h"
#include <SDL2/SDL.h>
namespace eclipse {

//////////////////////////////////////////////////////////
/////////// PUBLIC ///////////////////////////////////////
//////////////////////////////////////////////////////////

engine &engine::Instance() {
  // we defined our private mInstance before hand , so we know that if our
  // engine instance is created we just return a reference to it and if not we
  // create a new engine
  if (!mInstance) {
    mInstance = new engine;
  }
  return *mInstance;
}

bool engine::Initialize() {
  /*
  engine::Initialize()
  // log manager part , ideally we want logmanager before everything else even
  our sdl initialize so
  // we can error out what caused sdl to fail with our logmanger otherwise it
  wont work will it
  bool engine initialize obviously if you look at the function returns a boolean
  output , so we make it return ret , we make the ret a false initially , then
  we check , first we create an sdl init with the flag (SDL_INIT_EVERYTHING) ,
  this just means that we are initializing SDL2 with everything its got and if
  its less than 0 which means something didnt work we error out with whatever
  the error was with SDL_GetError() and if this didnt happen we do an else to
  print out the version , but how sdl deals with versin is through the
  SDL_VERSION macro , so we create an sdl version called version then we pass it
  in as a reference to SDL_VERSION where it populates it with the  " major " "
  minor " and " patch " well thats all fine but why do we cast to int for each
  of the version.major minor and patch , because we get ascii characters , if we
  dont cast it into ( to be specific its int32_t ) int
 now that we have window we can just add this to it
  */
  // mLogManager.Initialize();
  ECLIPSE_ASSERT(!mIsInitialized,
                "Attempting to call engine::Initialize() more than once ");
  bool ret = false;
  getInfo();
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    ECLIPSE_ERROR("Error initializing SDL2 : {}", SDL_GetError());
  } else {
    SDL_version version;
    SDL_VERSION(&version);
    ECLIPSE_INFO("SDL {}.{}.{} ", (int)version.major, (int)version.minor,
                 (int)version.patch);


    // after creating the windowproperties getter in app we need to pass it in
    // with a varaible called props to our mWindow.create and it will change depending
    // on what props we are passing in whether its overriden props or just regular props
    
    // important you keep forgetinggg thisk
    core::WindowProperties props = mApp ->GetWindowProperties();
    if (mWindow.Create(props)) {
      // initialze managers
      mRenderManager.Initialize();
      ret = true;
      mIsRunning = true;
      mIsInitialized = true;
      // input
      input::mouse::Initialize();
      input::keyboard::Initialize();
      // initialize client
      mApp->Initialize();
    }
  }
  if (!ret) {
    ECLIPSE_ERROR("Engine initialization failed {} ", SDL_GetError());
    Shutdown();
  }
  return ret;
}

void engine::Run(App *app) {
  mLogManager.Initialize();
  ECLIPSE_ASSERT(
      !mApp, "Attempting to call Engine::Run  when a valid app already exists");
  if (mApp) {
    return;
  }
  mApp = app;

  if (Initialize()) {
    while (mIsRunning) {
      mRenderManager.SetWireFrameMode(false);
      Update();
      Render();
    }
  }
  Shutdown();
}

void engine::Shutdown() {
  mIsRunning = false;
  mIsInitialized = false;
  // shutdown client
  mApp->Shutdown();
  mWindow.Shutdown();
  mLogManager.Shutdown();
  mRenderManager.Shutdown();
  SDL_Quit();
  // design note for later , initialzing and shutting down are done in reverse
  // order which means that if you initialize say logmanger first u should
  // probably shutdown it last
}

void engine::Quit() { mIsRunning = false; }
//////////////////////////////////////////////////////////
/////////// PRIVATE ///////////////////////////////////////
//////////////////////////////////////////////////////////

void engine::Update() {
  mWindow.PollEvents();
  mApp->Update();
}

void engine::Render() {
  mWindow.BeginRender();
  mApp->Render();
  mWindow.EndRender();
}

void engine::getInfo() {

  ECLIPSE_TRACE("eclipse v{}.{} ", 0, 1);

#ifdef ECLIPSE_CONFIG_DEBUG
  ECLIPSE_DEBUG("Eclipse configuration: DEBUG");
#endif

#ifdef ECLIPSE_CONFIG_RELEASE
  ECLIPSE_DEBUG("Eclipse configuration: RELEASE");
#endif

#ifdef ECLIPSE_PLATFORM_MAC
  ECLIPSE_WARN("Eclipse configuration: MAC");
#endif

#ifdef ECLIPSE_PLATFORM_LINUX
  ECLIPSE_WARN("Eclipse configuration: LINUX");
#endif

#ifdef ECLIPSE_PLATFORM_WINDOWS
  ECLIPSE_WARN("Eclipse configuration: WINDOWS");
#endif
}

// singleton ///

engine *engine::mInstance = nullptr;
// we create an mInstance so that our instance is private by default and only we
// can change our instance or create a new one with a getter method with our
// engine::Instance

// constructor
engine::engine() : mApp(nullptr), mIsRunning(false), mIsInitialized(false) {
  // getInfo();
  // this wont really work now since we have subsystems for turning on
  // logmanager
}

// everytime we call our engine we print out our get info
// i stupidly tried to print out void getInfo() thinking where i went wrong
// just a note for later , dont do that.

} // namespace eclipse
