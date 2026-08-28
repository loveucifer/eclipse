#include "../src/engine.h"
#include "../src/log.h"
#include "SDL2/SDL_error.h"
#include "SDL2/SDL_events.h"
#include "SDL2/SDL_video.h"
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include "../input/mouse.h"
#include "../input/keyboard.h"
#include "external/imgui/imgui.h"
#include "../src/app.h"

namespace eclipse::core{
  window::window():mWindow(nullptr){
    
  }

  window::~window(){
    if(mWindow){
      Shutdown();
    }
  }


  void window::GetSize(int& w , int& h){
    SDL_GetWindowSize(mWindow, &w, &h);
  }


  bool window::Create(){

    // we create the window with sdlcreatewindow , and accepts some parameters as following
    // x and y is the position we want it to be in we can just make it sdlwindowposcentered
    // so it will auto figure it out , title is just well title , then width and height , and flags
    // like sdlwindowborderless and then its width and height of the window , there is a cool flag i like
    // called sdl windowborderless that removes the border but it takes away the title a well but nvm , we also
    // have stuff like opengl and vulkan flags , maybe late

    mWindow =
        SDL_CreateWindow("eclipse", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);

    if (!mWindow) {
      ECLIPSE_ERROR("Error initializing SDL: {}" , SDL_GetError());
      return false;
    }

// after our sdlwindow is created we need to do our OpenGL calls but before doing alldat , we need some
// flags so that opengl works fine for us


#ifdef ECLIPSE_PLATFORM_MAC
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
// apple compatibility since they stopped supporting opengl
#endif
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,4 );
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  // double buffering is a technique where we do all the draw calls to
  // the back buffer and then we tell opengl to swap those buffers
  // and this helps with flickering and stuff
  SDL_SetWindowMinimumSize(mWindow, 200, 200);

  // now that we created all the attributes we want we can actually call opengl stuff

  mGLContext = SDL_GL_CreateContext(mWindow);
  if(mGLContext == nullptr){
    ECLIPSE_ERROR("Error initialzing SDL OpenGL context: {}" ,SDL_GetError());
    return false;
  }

  gladLoadGLLoader(SDL_GL_GetProcAddress);
  //  this loads all the gl versions and it maps the function pointers
  // that opengl makes to our graphics card and just maps whatever to whatever


  ///////////
  // IMGUI///
  ///////////

    mImGuiWindow.Create(); //
    // we dont want to render for each frame inside a while loop
    // that seems foolishg need to do soemthing else

  

    
  // move to renderer

  // glEnable(GL_DEPTH_TEST);
  // glDepthFunc(GL_LEQUAL);
  // glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); // blend things together
  // glClearColor(255,0,0,255);


    return true;


  }


void window::BeginRender(){
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the cached depth or color info
  engine::Instance().GetRenderManager().Clear();  // clearing it from our new manager class
}


void window::EndRender(){
  mImGuiWindow.BeginRender();

  // imgui calls
    // ImGui::ShowDemoWindow();
    engine::Instance().GetApp().ImGuiRender();
    

  mImGuiWindow.EndRender();
  SDL_GL_SwapWindow(mWindow);
}

void window::PollEvents(){
  SDL_Event event;
  // struct to capture os events
  while(SDL_PollEvent(&event)){
  mImGuiWindow.HandleSDLEvents(event);
    switch(event.type){
      case SDL_QUIT:
        engine::Instance().Quit();
        break;

        // the issue is that we have nothing to turn off the engine now
        // other than maybe shutdown but its private so we cant just access it like that
        // so we add a concept of isRunning to check whether if our engine is running
        // then we can just poll the while loop if its is running instead of just true
      default:
        break;
    }

  }

  // after polling the events update input
    input::mouse::Update();  
    input::keyboard::Update();
  
}


  void window::Shutdown(){
      SDL_DestroyWindow(mWindow);
      SDL_GL_DeleteContext(mGLContext);
      mWindow = nullptr;    
  }
  
}
