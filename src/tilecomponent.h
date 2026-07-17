#ifndef TILECOMPONENT_H
#define TILECOMPONENT_H
#include "SDL_render.h"
#include "entitymanager.h"
#include "component.h"
#include "assetmanager.h"
#include <glm/glm.hpp>

class TileComponent : public Component {
  public:
    SDL_Texture *texture;
    SDL_Rect sourceRectangle;
    SDL_Rect destinationRectangle;
    glm::vec2 position; // for x and y

  TileComponent(int sourceRectangleX, int sourceRectangleY, int x , int y , int tileSize , int tileScale , std::string assetTextureId){
    texture = Game::assetmanager -> GetTexture(assetTextureId);
    sourceRectangle.x = sourceRectangleX;
    sourceRectangle.y = sourceRectangleY;
    sourceRectangle.w = tileSize;
    sourceRectangle.h = tileSize;
    destinationRectangle.x = x;
    destinationRectangle.y = y;
    destinationRectangle.w = tileSize * tileScale;
    destinationRectangle.h = tileSize * tileScale;

    position.x = x;
    position.y = y;
  }

  ~TileComponent(){
    SDL_DestroyTexture(texture);
  }

  void Update(float deltaTime) override{
    // TODO we will fix tileposition here based on camera control
  }

  void Render() override {
    TextureManager::Draw(texture, sourceRectangle, destinationRectangle,SDL_FLIP_NONE );
  }


};



#endif
