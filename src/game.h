#ifndef GAME_H
#define GAME_H
#include "SDL_events.h"
#include "SDL_video.h"
#include <SDL2/SDL.h> // sdl from sdl2
#include <SDL2/SDL_image.h> // image from sdl2
#include <SDL2/SDL_ttf.h> // ttfs for fonts from sdl2


class AssetManager;

class Game {
  private:
    bool isRunning;
    // check if our game is actually running or nah 
    SDL_Window *window;
    // SDL_Window is a struct name for windows in SDL library
    // we initalize a pointer to this

 public:
   Game();
   ~Game();
   // constructor and destructor for out Game class
    bool GetIsRunning() const;  // obviously this is a getter
    // GetIsRunning is a const because the value does not obiously change for the getter function in this case
    void Initialize (int width , int height);
    // intializing with the parameters window width and heihgt which we setup in cosntants.h
    void LoadLevel(int level); // to load level we want 
    static SDL_Renderer *renderer;
    static AssetManager* assetmanager;
    static SDL_Event event;
    // static event is needed to poll the events for keebcontrol 
    static SDL_Rect camera;
    void HandleCameraMovement();
    // camera is basically a rectangle 

    void CheckCollisions();    
    // SDL_Renderer is a struct name for renderer in SDL library and same as
    // before we init a pointer to this
    // made it static to use it within transfomr component
    void ProcessNextLevel(int levelNumber);
    void ProcessGameOver();


    int TicksLastFrame;
    void ProcessInput();
    void Update();
    void Render();
    void Destroy();
    // the functions for the game loop that includes processing input updating it
    // rendering it and finally destory it

};

#endif
