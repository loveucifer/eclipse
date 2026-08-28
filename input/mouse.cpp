#include "mouse.h"
#include "../src/log.h"
#include "SDL2/SDL_mouse.h"
#include <algorithm>
#include <cstdint>


namespace eclipse::input{



int mouse::x = 0;
int mouse::xLast = 0;
int mouse::y = 0;
int mouse::yLast = 0;


// std::array<bool, mouse::ButtonCount> mouse::buttons;
// std::array<bool, mouse::ButtonCount> mouse::buttonsDown;
// std::array<bool, mouse::ButtonCount> mouse::buttonsUp;


std::array<bool, mouse::ButtonCount> mouse::buttons;
std::array<bool, mouse::ButtonCount> mouse::buttonsLast;

void mouse::Initialize(){
  std::fill(buttons.begin(), buttons.end(), false);
  std::fill(buttonsLast.begin(), buttonsLast.end(), false);

}

void mouse::Update(){
  
  uint32_t state = SDL_GetMouseState(&x, &y);

  xLast = x;
  yLast = y;


  buttonsLast = buttons; // todo optimize is memccpy faster

  for(int i =0 ;i < ButtonCount; i++){

    buttons[i] = state& SDL_BUTTON(i + 1);
    
    }
  }



// bounds check

bool mouse::Button(int button){
  ECLIPSE_ASSERT(button >= ECLIPSE_INPUT_MOUSE_FIRST && button <= ECLIPSE_INPUT_MOUSE_LAST, "Invalid mouse button");
  if(button >= ECLIPSE_INPUT_MOUSE_FIRST && button <= ECLIPSE_INPUT_MOUSE_LAST){
    return buttons[button -1];
   }
   return false;
}


bool mouse::ButtonUp(int button){
  
  ECLIPSE_ASSERT(button >= ECLIPSE_INPUT_MOUSE_FIRST && button <= ECLIPSE_INPUT_MOUSE_LAST, "Invalid mouse button");
  if(button >= ECLIPSE_INPUT_MOUSE_FIRST && button <= ECLIPSE_INPUT_MOUSE_LAST){
    return !buttons[button - 1] && buttonsLast[button -1];
   }
   return false;
}


bool mouse::ButtonDown(int button){
  
  ECLIPSE_ASSERT(button >= ECLIPSE_INPUT_MOUSE_FIRST && button <= ECLIPSE_INPUT_MOUSE_LAST, "Invalid mouse button");
  if(button >= ECLIPSE_INPUT_MOUSE_FIRST && button <= ECLIPSE_INPUT_MOUSE_LAST){
    return buttons[button - 1] && !buttonsLast[button -1];
   }
   return false;
}


} // namespace eclipse::input
