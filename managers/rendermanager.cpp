#include "rendermanager.h"
#include "../src/log.h"
#include <cstdint>
#include <glad/glad.h>
#include "../src/engine.h"
namespace eclipse::managers{

void RenderManager::Initialize(){
  ECLIPSE_INFO(
      "OpenGL info: \n  Renderer: \t{}\n Vendor: \t{}\n Version: \t{}",
      (const char *)glGetString(GL_RENDERER),
      (const char *)glGetString(GL_VENDOR),
      (const char *)glGetString(GL_VERSION));


  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0,0,1,1);
  
}
void RenderManager::Shutdown(){
  while(mRenderCommands.size()>0){
    mRenderCommands.pop();
  }
  while(mFrameBuffer.size()>0){
    mFrameBuffer.pop();
  }
}
void RenderManager::Clear(){
  ECLIPSE_ASSERT(mRenderCommands.size() == 0 , "unflushed render commands in queue");
  while(mRenderCommands.size()>0){
    mRenderCommands.pop();
  }
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void RenderManager::SetClearColor(const glm::vec4 clearColor){
  glClearColor(clearColor.r,clearColor.g,clearColor.b,clearColor.a);
}
void RenderManager::Submit(std::unique_ptr<graphics::rendercommands::RenderCommands> rc){
  mRenderCommands.push(std::move(rc));
}
void RenderManager::Flush(){

  while(mRenderCommands.size()>0){
  auto rc = std::move(mRenderCommands.front());
  mRenderCommands.pop();
  rc ->Execute();
  }
}
void RenderManager::SetWireFrameMode(bool enabled){
  if(enabled){
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  }else{
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  }
}

void RenderManager::SetViewPort(const glm::ivec4 dimension){
  glViewport(dimension.x,dimension.y,dimension.z,dimension.w);
}
void RenderManager::PushFrameBuffer(std::shared_ptr<graphics::FrameBuffer> frameBuffer){
  mFrameBuffer.push(frameBuffer);
  SetViewPort({0,0,frameBuffer->GetSize().x,frameBuffer->GetSize().y});
  glBindFramebuffer(GL_FRAMEBUFFER,frameBuffer->GetFbo());

  auto clearColor = frameBuffer->GetClearColor();
  glClearColor(clearColor.r,clearColor.g,clearColor.b,clearColor.a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void RenderManager::PopFrameBuffer(std::shared_ptr<graphics::FrameBuffer>){
  if(mFrameBuffer.size()>0){
    mFrameBuffer.pop();
  }
  if(mFrameBuffer.size()>0){
    auto nextFrameBuffer = mFrameBuffer.top();
    glBindFramebuffer(GL_FRAMEBUFFER,nextFrameBuffer->GetFbo());
    SetViewPort({0,0,nextFrameBuffer->GetSize().x,nextFrameBuffer->GetSize().y});
  }else{
    auto& window = Engine::Instance().GetWindow();
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    SetViewPort({0,0,window.GetSize().x,window.GetSize().y});
  }
}

} // namespace eclipse::managers
