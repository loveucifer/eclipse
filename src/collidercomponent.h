#ifndef COLLIDER_COMPONENT_H
#define COLLIDER_COMPONENT_H
#include "component.h"
#include "entitymanager.h"
#include "transformcomponent.h"
#include "game.h"

class ColliderComponent : public Component {
public:
  std::string colliderId;
  SDL_Rect collider;
  SDL_Rect sourceRectangle;
  SDL_Rect destinationRectangle;
  TransformComponent *transform;

  ColliderComponent(std::string colliderId, int x, int y, int width,
                    int height) {

    this->colliderId = colliderId;
    this->collider = {x, y, width, height};
  }

  void Initialize() override {

    if (owner -> HasComponent<TransformComponent>()){

    transform = owner->GetComponent<TransformComponent>();
    sourceRectangle = {0, 0, transform->width, transform->height};
    destinationRectangle = {collider.x, collider.y, collider.w, collider.h};
    }
  }

  void Update(float deltaTime) override {

    // collider component needs to udpate with the transform of the player
    // so collider x and y to the transform of player x and y

    collider.x = static_cast<int>(transform -> position.x);
    collider.y = static_cast<int>(transform -> position.y);
    collider.w = transform -> width * transform -> scale;
    collider.h = transform -> height * transform -> scale;
    destinationRectangle.x = collider.x - Game::camera.x;
    destinationRectangle.y = collider.y - Game::camera.y;
    
  }
};

#endif
