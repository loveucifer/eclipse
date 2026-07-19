#ifndef CONSTANTS_H
#define CONSTANTS_H

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
   GUI_LAYER = 5
};

const unsigned int NUM_LAYER = 6;

#endif
