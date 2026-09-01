#pragma once

#include <SDL_events.h>

typedef union SDL_Event SDL_Event;

namespace eclipse::core{

  struct ImguiWindowProperties{
    bool MoveFromTitleBarOnly = true;
    bool IsDockingEnabled = false;
    bool IsViewPortEnabled = false;
  };



  class ImguiWindow{
    public:
      ImguiWindow(){}
      ~ImguiWindow(){}
      void Create(const ImguiWindowProperties& props);
      void Shutdown();

      void HandleSDLEvents(SDL_Event& event);

      bool WantCaptureMouse();
      bool WantCaptureKeyboard();
      void BeginRender();
      void EndRender();
      

    private:
      bool mDockingEnabled = false;
  };
}
