#include "texturemanager.h"
#include "SDL_render.h"
#include "SDL_surface.h"

SDL_Texture *TextureManager::LoadTexture(const char *fileName){
  SDL_Surface *surface = IMG_Load(fileName);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(Game::renderer,surface);
  SDL_FreeSurface(surface);
  return texture;

  ///////////////////////////////////////////////////
  // going over this function we create something called loadtexture
  // which loads texture from a filename , but for it to actually load
  // we need an sdl surface so we create a sdl surface and then load the image
  // then we create the texture from the surface which has our file and then
  // finally freee the surface and return our texture//////////
  /////////////////////////////////////////////////////////////////

}

void TextureManager::Draw(SDL_Texture *texture, SDL_Rect sourceRectangle,
                          SDL_Rect destinationRectangle,
                          SDL_RendererFlip flip) {

  SDL_RenderCopyEx(Game::renderer, texture, &sourceRectangle, &destinationRectangle, 0.0, NULL,  flip);
}
