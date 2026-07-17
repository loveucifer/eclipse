#ifndef SPRITECOMPONENT_H
#define SPRITECOMPONENT_H
#include <SDL2/SDL.h>
#include "SDL_render.h"
#include "assetmanager.h"
#include "texturemanager.h"
#include "transformcomponent.h"
#include "animation.h"


class SpriteComponent: public Component {
  private:
      TransformComponent *transform;
      SDL_Texture *texture;
      SDL_Rect sourceRectangle;
      SDL_Rect destinationRectangle;
      bool isAnimated;
      bool isStatic; // static as in it stays in the same place
      int numFrames;
      int animationSpeed;
      std::map<std::string,Animation> animations; // tracks animations
      std::string currentAnimationName;
      unsigned int animationIndex = 0;

      

      


  public:
      SDL_RendererFlip spriteFlip = SDL_FLIP_NONE;
      SpriteComponent(std::string assetTextureId){
        isAnimated = false;
        isStatic = false;
        setTexture(assetTextureId);

      }


      SpriteComponent(std::string id,int numFrames,int animationSpeed,bool hasDirection,bool isStatic){

        isAnimated = true;
        this->numFrames = numFrames;
        this->animationSpeed = animationSpeed;
        this->isStatic = isStatic;

        if (hasDirection) {
          Animation downAnimation = Animation(0, numFrames, animationSpeed);
          Animation rightAnimation = Animation(1, numFrames, animationSpeed);
          Animation leftAnimation = Animation(2, numFrames, animationSpeed);
          Animation upAnimation = Animation(3, numFrames, animationSpeed);

          animations.emplace ("DownAnimation" ,downAnimation);
          animations.emplace ("RightAnimation",rightAnimation);
          animations.emplace ("LeftAnimation",leftAnimation);
          animations.emplace ("UpAnimation",upAnimation);

          this -> animationIndex = 0;
          this -> currentAnimationName ="DownAnimation";


        } else {
          Animation singleAnimation = Animation(0, numFrames, animationSpeed);
          animations.emplace("SingleAnimation", singleAnimation);
          this->animationIndex = 0;
          this->currentAnimationName = "SingleAnimation";
        }

          Play(this->currentAnimationName);
          setTexture(id);
        
      }

      void Play(std::string animationName){
        numFrames = animations[animationName].numFrames;
        animationIndex = animations[animationName].index;
        animationSpeed = animations[animationName].animationSpeed;
      }

      
      void setTexture(std::string assetTextureId) {
        texture = Game::assetmanager->GetTexture(assetTextureId);
      }

      void Initialize() override {
        transform = owner ->GetComponent<TransformComponent>();
        sourceRectangle.x = 0;
        sourceRectangle.y = 0;
        sourceRectangle.w = transform -> width;
        sourceRectangle.h =  transform -> height;
      }

      void Update (float deltaTime) override {

        if (isAnimated) {
          sourceRectangle.x = sourceRectangle.w * static_cast<int>(                               (SDL_GetTicks() / animationSpeed) % numFrames);
        }

        sourceRectangle.y = animationIndex * transform -> height;

        destinationRectangle.x = static_cast<int>(transform -> position.x);
        destinationRectangle.y = static_cast<int>(transform -> position.y);
        destinationRectangle.w = transform->width * transform->scale;
        destinationRectangle.h = transform->height * transform->scale;
      }

      void Render () override {
        TextureManager::Draw(texture, sourceRectangle, destinationRectangle, spriteFlip);
      }

};



#endif
