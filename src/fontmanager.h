#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H
#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_ttf.h"
#include "game.h"
#include <SDL2/SDL.h>


class FontManager{
  public:
    static TTF_Font *LoadFont(const char *fileName,int fontSize);
    static void Draw(SDL_Texture *texture,SDL_Rect position);
};




#endif
