#pragma once
#include <memory>
#include <queue>
#include <stack>
#include "../graphics/rendercommands.h"
#define ECLIPSE_SUBMIT_RC(type, ...)                                           \
  std::make_unique<eclipse::graphics::rendercommands::type>(__VA_ARGS__)
namespace eclipse::managers{
  class RenderManager{
    friend class graphics::rendercommands::PopFrameBuffer;
    friend class graphics::rendercommands::PushFrameBuffer;
    public:
      RenderManager(){}
      ~RenderManager(){}
       void Initialize();
       void Shutdown();
       void Clear();
       void SetClearColor(const glm::vec4 clearColor);
       void Submit(std::unique_ptr<graphics::rendercommands::RenderCommands>rc);
       void Flush();
       void SetWireFrameMode(bool enabled);
       void SetViewPort(const glm::ivec4 dimension);
       void PushFrameBuffer(std::shared_ptr<graphics::FrameBuffer>frameBuffer);
       void PopFrameBuffer(std::shared_ptr<graphics::FrameBuffer>frameBuffer);
    private:
      std::queue<std::unique_ptr<graphics::rendercommands::RenderCommands>>mRenderCommands;
      std::stack<std::shared_ptr<graphics::FrameBuffer>>mFrameBuffer;

  };
}
