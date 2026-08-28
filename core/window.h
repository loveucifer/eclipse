#pragma once
#include "imguiwindow.h"
#include "SDL2/SDL_video.h"
struct SDL_Window;
using SDL_GLContext = void*;

namespace eclipse::core{
  class window{
    public:
      window();
      ~window();

      bool Create();

      void Shutdown();

      void PollEvents();

      void BeginRender();
      void EndRender();


      void GetSize(int& w, int& h);

      inline SDL_Window* GetSDLWindow(){return mWindow;}

      
      // we dont do a pointer here because its already a pointer wiht our void* on line 5
      inline SDL_GLContext GetSDLGLContext(){return mGLContext;}
    private:


      SDL_Window* mWindow; // sdl gives us a pointer to the window
      SDL_GLContext mGLContext;
       // already a void* no need for further pointers 
      ImguiWindow mImGuiWindow;
  };
}
