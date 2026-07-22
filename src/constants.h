#ifndef CONSTANTS_H
#define CONSTANTS_H
#include "SDL2/SDL.h"

const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 600;

const unsigned int FPS = 60;
const unsigned int FRAME_TARGET_TIME = 1000/FPS;

// frame target time is the time taken by a frame to render
// fps is the frames per second 

enum LayerType {
   TILEMAP_LAYER = 0,
   VEGETATION_LAYER = 1,
   ENEMY_LAYER = 2,
   PLAYER_LAYER = 3,
   PROJECTILE_LAYER = 4,
   GUI_LAYER = 5,
   OBSTACLE_LAYER = 6
};

enum CollisionType {
  NO_COLLISION,
  PLAYER_ENEMY_COLLISION,
  PLAYER_PROJECTILE_COLLISION,
  PLAYER_VEGETATION_COLLISION,
  PLAYER_LEVEL_COMPLETE_COLLISION,
  ENEMY_PROJECTILE_COLLISION
};

const unsigned int NUM_LAYER = 7;

const SDL_Color WHITE_COLOR = {255,255,255,255};
const SDL_Color GREEN_Color = {0,255,0,255};
#endif
