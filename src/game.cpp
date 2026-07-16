#include "./game.h"
#include "./constants.h"
#include "SDL.h"
#include "SDL_events.h"
#include "SDL_keyboard.h"
#include "SDL_keycode.h"
#include "SDL_render.h"
#include "SDL_timer.h"
#include "glm/glm.hpp"
#include "SDL_video.h"
#include <iostream>
#include "entitymanager.h"
#include "transformcomponent.h"

EntityManager manager;
SDL_Renderer *Game::renderer;


// constructor
Game::Game() { this->isRunning = false; }

// destructor
Game::~Game() {}

bool Game::GetIsRunning() const { return this->isRunning; }


// welll this was a naive way to deal with this
// we should use vector instead from glm
// float projectilePositionX = 0.0f;
// float projectilePositionY = 0.0f;
// float projectileVelocityX = 10.0f;
// float projectileVelocityY = 10.0f;

// glm::vec2 projectilePosition = glm::vec2(0.0f,0.0f);
// glm::vec2 projectileVelocity = glm::vec2(10.0f,10.0f);

// basically we just replaced our floats with vectors from glm

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

LoadLevel(0);
isRunning = true;
return;
}

void Game::LoadLevel(int levelNumber) {

  Entity &newEntity(manager.AddEntity("projectile"));
  newEntity.AddComponent<TransformComponent>(0, 0, 20, 20, 32, 32, 1);
  Entity &weirdEntity(manager.AddEntity("WEIRD"));
  weirdEntity.AddComponent<TransformComponent>(100,100,20,20,32,32,1);
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


/////////////////////////////////////////////////////
/////////////////// DELTA TIME //////////////////////
// the thing about using values like this is that after
// we use them they run differently based on different
// specifications on the end users machine so for some
// the projectile must be faster , for some it may be slow
// we cant really have that we need to clip this somehow
// and that is where delta time comes in , delta stands for change
// or difference in something , so we use that to make it same for all the users
// and how we do that is with this piece of code
// float deltaTime = (SDL_GetTicks() - ticksLastFrame/1000.0f)
// SDL_Getticks polls the necessary ticks we need
// delta time is the differnece between ticks from last frame converted to seconds
//////////////////////////////////////////////////////////////////

void Game::Update(){

  // wait until taget time  (16ms) has elapsed

 // while(!SDL_TICKS_PASSED(SDL_GetTicks(), TicksLastFrame + FRAME_TARGET_TIME));

// while loops are cpu processes so they eat up unnecssary compute we actually want rather replace it with sdl delay

int TimeToWait = FRAME_TARGET_TIME - (SDL_GetTicks() - TicksLastFrame);
if (TimeToWait > 0 && TimeToWait <= static_cast<int>(FRAME_TARGET_TIME)){
  SDL_Delay(TimeToWait);
}


Uint32 currentTicks = SDL_GetTicks();
float deltaTime = (currentTicks - TicksLastFrame) / 1000.0f;
deltaTime = (deltaTime > 0.05f) ? 0.05f : deltaTime;
TicksLastFrame = currentTicks;

// projectilePositionX += projectileVelocityX *deltaTime;
// projectilePositionY += projectileVelocityY *deltaTime;
// projectilePosition = glm::vec2(

//   projectilePosition.x += projectileVelocity.x *deltaTime,
//   projectilePosition.y += projectileVelocity.x *deltaTime
// );
//

manager.Update(deltaTime);

}

  ///////////////////////////////////////////////////////////////////////
  ///////// Double Buffering -> Back Buffering and Front Buffering //////
  ///////////////////////////////////////////////////////////////////////
  // sdl2 uses double buffering which means it has 2 buffers which consists of
  // front and back buffer , what this means for us is that we clear the buffer
  // draw all the game objects and then we swap the front and back buffers
  // here first we set sdl set renderdraw color where we draw all game objects then we do sdl render present renderer which swaps the front and back buffers

void Game::Render() {
  SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
  // SetRenderDrawColor accepts 4 arguments
  // which are quite easy to figure out they are rgba
  // r for red g for green b for blue and a for alpha which is opacity
  SDL_RenderClear(renderer);
  // SDL_Rect projectile {
  //  (int)projectilePosition.x,
  //  (int)projectilePosition.y,
  //  10,
  //  10
  // };

  // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  // SDL_RenderFillRect(renderer, &projectile);

  if (manager.HasNoEntities()) {
    return;
  }

  manager.Render();

  SDL_RenderPresent(
      renderer); // this swaps the front and back buffers ( check comment above)
}

/////////////////////////////////////////////////////////////////////////////////////
// to put these functions into perspective we have Game::Update which is our
// update function we make it so that the projectile postiion x gets added with
// the velocity for each axis then we ask the Game::Render which is our renderer
// fucntion to go and draw these points for that first we clear out render
// buffer which is not necessary btw i just did it so , then we ask it to draw
// our projectile it accepts 4 arguments which are x and y position and width
// and height , then we setup the draw color which accepts rgba which we set all
// to maximum and then we fill the rectangle with the colours , the fill
// rectangle accpets an argument for renderre and a pointer to where we want
// this to be drawn thiwhc is a pointer to our projectile
///////////////////////////////////////////////////////////////////////////////////////

void Game::Destroy(){
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
