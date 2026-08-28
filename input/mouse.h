#pragma once
#include "SDL2/SDL_mouse.h"
#include <SDL2/SDL.h>
#include <array>


namespace eclipse::input{
  class mouse{
    public:

      static void Initialize();

      static void Update();
      inline static int X (){return x;}
      inline static int Y (){return y;}

      inline static int DX(){return x- xLast;}
      inline static int DY(){return y - yLast;}
      static bool Button(int button);
      static bool ButtonDown(int button);
      static bool ButtonUp(int button);

    private:
      constexpr static const int ButtonCount =5; // sdl supports 5 mouse buttons
      static std::array<bool,ButtonCount>buttons;
      static std::array<bool,ButtonCount>buttonsLast;
      

      static int x,xLast;
      static int y,yLast;
      
  };


// adapted from sdl  see sdl button left
enum {
  ECLIPSE_INPUT_MOUSE_FIRST = 1,
  ECLIPSE_INPUT_MOUSE_LEFT = ECLIPSE_INPUT_MOUSE_FIRST,
  ECLIPSE_INPUT_MOUSE_MIDDLE = 2,
  ECLIPSE_INPUT_MOUSE_RIGHT = 3,
  ECLIPSE_INPUT_MOUSE_X1= 4,
  ECLIPSE_INPUT_MOUSE_X2= 5,
  ECLIPSE_INPUT_MOUSE_LAST = 5

};

  
}
