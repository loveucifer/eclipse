#pragma once
#include <memory>
#include <queue>
#include "../graphics/rendercommands.h"
namespace eclipse::managers{
  class RenderManager{
    public:
      RenderManager(){}
      ~RenderManager(){}

       void Initialize();
       void Shutdown();

       void Submit(std::unique_ptr<graphics::rendercommands::RenderCommand> rc);

        void Clear();

       void SetClearColor(float r , float g , float b , float a);

       void SetWireFrameMode(bool enabled);


      // execute render commands in the order ( no sorting )
       void Flush();
    private:
      
      std::queue<std::unique_ptr<graphics::rendercommands::RenderCommand>> mRenderCommands;
  };
}
