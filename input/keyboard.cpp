#include "keyboard.h"
#include "../src/log.h"
#include "SDL2/SDL_keyboard.h"
#include "SDL2/SDL_stdinc.h"
#include <algorithm>

namespace eclipse::input{


std::array<bool, keyboard::KeyCount> keyboard::keys;
std::array<bool, keyboard::KeyCount> keyboard::keysLast;
bool keyboard::isEnabled = true;



void keyboard::Initialize(){
  std::fill(keys.begin(), keys.end(), false);

  std::fill(keysLast.begin(), keysLast.end(), false);
  isEnabled = true;

}


void keyboard::Update(){

keysLast = keys;
  
const Uint8* state = SDL_GetKeyboardState(NULL);


  for(int i =ECLIPSE_INPUT_KEY_FIRST ;i < KeyCount; i++){
    keys[i] = state [i];  // gets an array

}
}
void keyboard::SetEnabled(bool enabled) {
  isEnabled = enabled;
}
// bounds check

bool keyboard::Key(int key){
  ECLIPSE_ASSERT(key >= ECLIPSE_INPUT_KEY_FIRST && key < KeyCount, "Invalid keyboard key");
  if(key >= ECLIPSE_INPUT_KEY_FIRST && key < KeyCount){
    return isEnabled && keys[key];
   }
   return false;
}


bool keyboard::KeyUp(int key){
   ECLIPSE_ASSERT(key >= ECLIPSE_INPUT_KEY_FIRST && key < KeyCount, "Invalid keyboard key");
  if(key >= ECLIPSE_INPUT_KEY_FIRST && key < KeyCount){
    return isEnabled && !keys[key] && keysLast[key];
   }
   return false;
}


bool keyboard::KeyDown(int key){
  
  ECLIPSE_ASSERT(key >= ECLIPSE_INPUT_KEY_FIRST && key < KeyCount, "Invalid keyboard key");
  if(key >= ECLIPSE_INPUT_KEY_FIRST && key < KeyCount ){
  
    return isEnabled && keys[key] && !keysLast[key];
   }
   return false;
}

bool keyboard::IsValidKey(int key) {
  return key >= ECLIPSE_INPUT_KEY_FIRST && key < KeyCount;
}


} // namespace eclipse::input
