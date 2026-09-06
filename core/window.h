#pragma once
#include "imguiwindow.h"
#include "SDL2/SDL_video.h"
#include <memory>
#include <string>
#include "../graphics/framebuffer.h"
#include "../glm/glm.hpp"
struct SDL_Window;
using SDL_GLContext = void*;

namespace eclipse::graphics{
  class FrameBuffer;
}


namespace eclipse::core{

    struct WindowProperties {
        std::string title;
        int x,y,w,h;
        int wMin,hMin;
        int flags;
        // float ccR,ccG,ccB; // clear color
        glm::vec3 clearColor;

        // to handle imgui properties passed in
        // from our imguiwindow.h

        ImguiWindowProperties ImGuiProps;
        

        WindowProperties(); 
      };

  class window{
    public:
      window();
      ~window();
      bool Create(const WindowProperties& props);

      void Shutdown();

      void PollEvents();

      void BeginRender();
      void EndRender();


      // void GetSize(int& w, int& h);
      glm::ivec2 GetSize(); //vector of 2 integers


      inline SDL_Window* GetSDLWindow(){return mWindow;}
      inline graphics::FrameBuffer* GetFrameBuffer(){return mFrameBuffer.get();}
      
      // we dont do a pointer here because its already a pointer wiht our void* on line 5
      inline SDL_GLContext GetSDLGLContext(){return mGLContext;}
    private:

      ImguiWindow mImGuiWindow;
      SDL_Window* mWindow; // sdl gives us a pointer to the window
      SDL_GLContext mGLContext;
       // already a void* no need for further pointers 
      std::shared_ptr<graphics::FrameBuffer>mFrameBuffer;
  };
}
