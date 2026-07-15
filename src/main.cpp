#include <iostream>
#include "./game.h"
#include "./constants.h"

int main(int argc , char *args[]){

  Game *game = new Game();
  game-> Initialize(WINDOW_WIDTH,WINDOW_HEIGHT);

// not to self earlier had a confusion between the isRunning and
// Isrunning and since this is a getter its obviously better to use
// GetIsRunning as it makes more sense of what it does and i dont try to access
// the private memeber of the Game class which is isRunning

  while(game -> GetIsRunning()){
    game -> ProcessInput();
    game -> Update();
    game -> Render();
  }
  game -> Destroy();
  return 0;
}

///////////////////////////////////////////////////
// the basic idea of a game loop is 3 things
// we process the input
// we update the game
// we render it
// this is a loop that keeps running
// thats about it.
//   we can think of this in code like
//
//   while (true) {
//    game-> processInputj();
//    game-> Update();  
//    game -> render();
// }
//////////////////////////////////////////////////
