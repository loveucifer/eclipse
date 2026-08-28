#include "rendermanager.h"
#include "../src/log.h"
#include <glad/glad.h>
#include "../graphics/helpers.h"
namespace eclipse::managers {



void RenderManager::SetWireFrameMode(bool enabled){

if(enabled){
  glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);ECLIPSE_CHECK_GL_ERROR;
}else{
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); ECLIPSE_CHECK_GL_ERROR;
}
}

void RenderManager::Initialize() {
  ECLIPSE_INFO("OpenGL info: \n Vendor: \t{}\n Renderer:\t{}\n  Version:\t{}",
               (const char *)glGetString(GL_VENDOR),
               (const char *)glGetString(GL_RENDERER),
               (const char *)glGetString(GL_VERSION));

  // iniitialize Opengl:
  glEnable(GL_DEPTH_TEST);ECLIPSE_CHECK_GL_ERROR;
  glDepthFunc(GL_LEQUAL);ECLIPSE_CHECK_GL_ERROR;
  glEnable(GL_BLEND);ECLIPSE_CHECK_GL_ERROR;
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); ECLIPSE_CHECK_GL_ERROR;// blend things together

  glClearColor(0, 0, 255, 0);ECLIPSE_CHECK_GL_ERROR;
}

void RenderManager::SetClearColor(float r, float g, float b, float a) {
  glClearColor(r, g, b, a);ECLIPSE_CHECK_GL_ERROR;
}

void RenderManager::Submit(
    std::unique_ptr<graphics::rendercommands::RenderCommand> rc) {
  mRenderCommands.push(std::move(rc));
}

void RenderManager::Shutdown() {
  while (mRenderCommands.size() > 0) {
    mRenderCommands.pop();
  }
}

void RenderManager::Clear() {

ECLIPSE_ASSERT(mRenderCommands.size() == 0, "Unflushed render commands in queue");


  while (mRenderCommands.size() > 0) {
    mRenderCommands.pop();
  }

  glClear(GL_COLOR_BUFFER_BIT |
          GL_DEPTH_BUFFER_BIT); // clear the cached depth or color info
}

void RenderManager::Flush() {
  while (mRenderCommands.size() > 0) {
    auto rc = std::move(mRenderCommands.front());
    mRenderCommands.pop();
    rc->Execute();
  }
}
} // namespace eclipse::managers
