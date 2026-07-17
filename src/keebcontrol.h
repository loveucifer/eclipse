#ifndef KEEBCONTROL_H
#define KEEBCONTROL_H
#include <iostream>
#include <string>
#include "SDL_events.h"
#include "game.h"
#include "entitymanager.h"
#include "component.h"
#include "transformcomponent.h"
#include "spritecomponent.h"


// keebcontrol is really a component so it inherits from component

class KeebControl: public Component {
  public:
    std::string upKey;
    std::string rightKey;
    std::string downKey;
    std::string leftKey;
    std::string shootKey;
    TransformComponent *transform;
    SpriteComponent *sprite;

    KeebControl() {
      
    }

    KeebControl(std::string upKey, std::string downKey, std::string rightKey,
                std::string leftKey, std::string shootKey) {
      this->upKey = GetSDLKey(upKey);
      this->downKey = GetSDLKey(downKey);
      this->rightKey = GetSDLKey(rightKey);
      this->leftKey = GetSDLKey(leftKey);
      this->shootKey = GetSDLKey(shootKey);
    }

    std::string GetSDLKey(std::string key) {
      if (key.compare("up") == 0)
        return "1073741906";
      if (key.compare("down") == 0)
        return "1073741905";
      if (key.compare("left") == 0)
        return "1073741904";
      if (key.compare("right") == 0)
        return "1073741903";
      if (key.compare("space") == 0)
        return "32";
      else
        return std::to_string(static_cast<int>(key[0]));
    }

    void Initialize () override{
      transform = owner -> GetComponent<TransformComponent>();
      sprite = owner -> GetComponent<SpriteComponent>();
    }

    void Update(float deltaTime) override {
      if (Game::event.type == SDL_KEYDOWN) {
        std::string keyCode = std::to_string(Game::event.key.keysym.sym);

        if (keyCode.compare(upKey) == 0) {
            transform->velocity.y = -200;
            transform -> velocity.x = 0;
            sprite -> Play("UpAnimation");
        }
        if (keyCode.compare(downKey) == 0) {
           transform -> velocity.y = 200;
           transform -> velocity.x = 0;
           sprite -> Play("DownAnimation");
        }
        if (keyCode.compare(rightKey) == 0) {
           transform -> velocity.x = 200;
           transform -> velocity.y = 0;
           sprite -> Play("RightAnimation");
        }
        if (keyCode.compare(leftKey) == 0) {
           transform -> velocity.x = -200;
           transform -> velocity.y = 0;
           sprite -> Play("LeftAnimation");
        }
        if (keyCode.compare(shootKey) == 0) {
            // for later i guess 
        }
      }

      if (Game::event.type == SDL_KEYUP){
        std::string keyCode = std::to_string(Game::event.key.keysym.sym);
        if (keyCode.compare(upKey) == 0) {

            transform->velocity.y =0;

        }
        if (keyCode.compare(downKey) == 0) {

            transform -> velocity.y = 0;

        }
        if (keyCode.compare(rightKey) == 0) {

            transform -> velocity.x = 0;

        }
        if (keyCode.compare(leftKey) == 0) {

            transform -> velocity.x = 0;

        }
        if (keyCode.compare(upKey) == 0) {
          // TODO move it up by transforming or something
        }
      }
    }

    void Render() override {}
};

#endif
