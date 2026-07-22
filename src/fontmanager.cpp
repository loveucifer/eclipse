#include "fontmanager.h"
#include "SDL_render.h"
#include "SDL_ttf.h"


TTF_Font *FontManager::LoadFont(const char *fileName, int fontSize) {
  return TTF_OpenFont(fileName, fontSize);
}

void FontManager::Draw(SDL_Texture *texture, SDL_Rect position) {
  SDL_RenderCopy(Game::renderer, texture, NULL,  &position);
  // sdl rectangle source is made null because we can use the entire
  // texture as source
  // the destination rectangle is a reference to the position rectangle we created before
}
