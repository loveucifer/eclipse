#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H
#include "SDL_render.h"
#include "entitymanager.h"
#include "game.h"
#include <glm/glm.hpp>
#include <vector>
#include <SDL2/SDL.h>

class TransformComponent : public Component {
public:
  glm::vec2 position;
  glm::vec2 velocity;
  int width;
  int height;
  int scale;

  TransformComponent(int positionX, int positionY, int velocityX, int velocityY,
                     int w, int h, int s) {
    position = glm::vec2(positionX, positionY);
    velocity = glm::vec2(velocityX, velocityY);
    height = h;
    width = w;
    scale = s;

  }

  void Initialize() override {}

  void Update(float deltaTime) override {
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
  }

  // we moved the update logic from component to transform component above

  void Render() override {
    SDL_Rect transformRectangle = {
      (int) position.x,
      (int) position.y,
      width,
      height
    };
    SDL_SetRenderDrawColor(Game::renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(Game::renderer, &transformRectangle);  
      }
};

#endif
