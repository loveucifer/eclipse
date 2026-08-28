#pragma once

#include <SDL_events.h>

typedef union SDL_Event SDL_Event;

namespace eclipse::core{
  class ImguiWindow{
    public:
      ImguiWindow(){}
      ~ImguiWindow(){}

      void Create();
      void Shutdown();
      void HandleSDLEvents(SDL_Event& event);
      void BeginRender();
      void EndRender();
    private:
  };
}
