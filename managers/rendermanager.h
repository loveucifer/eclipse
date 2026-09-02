#pragma once
#include <memory>
#include <queue>
#include <stack>
#include "../graphics/rendercommands.h"

#define ECLIPSE_SUBMIT_RC(type,...)std::move(std::make_unique<eclipse::graphics::rendercommands::type>(__VA_ARGS__))

namespace eclipse::managers{
  class RenderManager{
      friend class graphics::rendercommands::PushFrameBuffer;
      friend class graphics::rendercommands::PopFrameBuffer;
    public:
      RenderManager(){}
      ~RenderManager(){}

       void Initialize();
       void Shutdown();

       void Submit(std::unique_ptr<graphics::rendercommands::RenderCommand> rc);

        void Clear();

        void SetViewport(int x, int y,int w, int h);

       void SetClearColor(float r , float g , float b , float a);

       void SetWireFrameMode(bool enabled);


      // execute render commands in the order ( no sorting )
       void Flush();

       private:
         void PushFrameBuffer(std::shared_ptr<graphics::FrameBuffer>framebuffer);
         void PopFrameBuffer(std::shared_ptr<graphics::FrameBuffer>framebuffer);
    private:
      
      std::queue<std::unique_ptr<graphics::rendercommands::RenderCommand>> mRenderCommands;
      std::stack<std::shared_ptr<graphics::FrameBuffer>>mFrameBuffer;

  };
}
