#include "./game.h"
#include "./constants.h"
#include "SDL.h"
#include "SDL_events.h"
#include "SDL_keyboard.h"
#include "SDL_keycode.h"
#include "SDL_render.h"
#include "SDL_video.h"
#include <iostream>

// constructor
Game::Game() { this->isRunning = false; }

// destructor
Game::~Game() {}

bool Game::GetIsRunning() const { return this->isRunning; }

float projectilePositionX = 0.0f;
float projectilePositionY = 0.0f;
float projectileVeloctiyX = 10.0f;
float projectileVeloctiyY = 10.0f;

// what happens here is we initalize everything for sdl not just the keyboard or
// some other thing

void Game::Initialize(int width, int height) {

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    std::cerr << "Error initlaizing SDL." << '\n';
    return;
  }

  window = SDL_CreateWindow(
      
      NULL,
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      width,
      height,
      SDL_WINDOW_BORDERLESS);

  // const char *title, int x, int y, int w, int h, Uint32 flags
  // sdl create window accepts a lot of parameters so lets go over it 1 by 1
  // first is called title basically the title of our window
  // next is the position in x and y units here we use SDL_WINDOWPOS_CENTERED
  // Whcih makes sure that we actually get it centered
  // then we have width and height represented by int w and h here we use our
  // constants from WINDOW_WIDTH AND WINDOW_HEIGHT but these are just passed as
  // is with width and height but just for reference for later :/ and last we
  // have flags
  
    if (!window) {
      std::cerr << "Error creating SDL window" << "\n";
      return;
    }

    renderer = SDL_CreateRenderer(
      window,
      -1,
      0);
    // renderer accepts 3 parameters
    // which are window , index and flags
    // window is oobiously our window
    // index is the display monitor
    // we can just say -1 which means default  

if (! renderer){
  std::cerr << "Error creating SDL renderer " << '\n';
  return;
}

isRunning = true;
return;

}

void Game::ProcessInput() {
  SDL_Event event;
  SDL_PollEvent(&event);
  switch (event.type) {
  case SDL_QUIT: {
    isRunning = false;
    break;
  }
  case SDL_KEYDOWN: {
    if (event.key.keysym.sym == SDLK_ESCAPE) {
      isRunning = false;
    }
  }

  default: {
    break;
  }
  }
}

///////////////////////////////////////////////////////////////////////////
// the Process input function does the part of accepting and
// processing the inputs given by the end user
// SDL_EVENT event creates a new event and we use SDL_POLLEvent
// to poll the created event but we access it via the address with the
// pointer to the event with &event whcih means we get the address of the
// created event and then poll the events for which we have some cases and we use
// switch here to assess for those cases which are for now if the user has decided
// to quit the game , for eg if the user clicks " ESCAPE " we have to quit so we use
// SDL_KEYDOWN to ensure that we do quit for this case 
/////////////////////////////////////////////////////////////////////////////



void Game::Update(){
  projectilePositionX += projectileVeloctiyX; 
  projectilePositionY += projectileVeloctiyY;
}

void Game::Render(){
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
  // SetRenderDrawColor accepts 4 arguments
  // which are quite easy to figure out they are rgba
  // r for red g for green b for blue and a for alpha which is opacity
  SDL_RenderClear(renderer);
  SDL_Rect projectile {
   (int)projectilePositionX,
   (int)projectilePositionY,
   10,
   10
  };

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderFillRect(renderer, &projectile);
  SDL_RenderPresent(renderer);
  
}


/////////////////////////////////////////////////////////////////////////////////////
// to put these functions into perspective we have Game::Update which is our
// update function we make it so that the projectile postiion x gets added with the
// velocity for each axis
// then we ask the Game::Render which is our renderer fucntion to go and draw these points for that first we clear out render buffer which is not necessary btw i just did it so , then we ask it to draw our projectile it accepts 4 arguments which are x and y position and width and height , then we setup the draw color which accepts rgba which we set all to maximum and then we fill the rectangle with the colours , the fill rectangle accpets an argument for renderre and a pointer to where we want this to be drawn thiwhc is a pointer to our projectile
///////////////////////////////////////////////////////////////////////////////////////


void Game::Destroy(){
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
