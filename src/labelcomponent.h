#ifndef LABEL_COMPONENT_H
#define LABEL_COMPONENT_H
#include "SDL_pixels.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_surface.h"
#include "SDL_ttf.h"
#include "entitymanager.h"
#include "assetmanager.h"
#include "game.h"
#include "entitymanager.h"
#include "fontmanager.h"

class LabelComponent : public Component {

private:
  SDL_Rect position;
  std::string text;
  std::string fontFamily;
  SDL_Color color;
  SDL_Texture *texture;

public:
  LabelComponent(int x, int y, std::string text, std::string fontFamily,
                 const SDL_Color &color) {
    this->position.x = x;
    this->position.y = y;
    this->text = text;
    this->fontFamily = fontFamily;
    this->color = color;
    SetLabelText(text, fontFamily);
  }

  void SetLabelText(std::string text, std::string fontFamily) {
    SDL_Surface *surface = TTF_RenderText_Blended(
        Game::assetmanager->GetFont(fontFamily), text.c_str(), color);
    texture = SDL_CreateTextureFromSurface(Game::renderer, surface);
    SDL_FreeSurface(surface);
    SDL_QueryTexture(texture, NULL, NULL, &position.w, &position.h);
  }


  void Update(float deltaTime) override {
    
  }

  void Render() override {
      FontManager::Draw(texture, position);
  }

  
};

#endif
