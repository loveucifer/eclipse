#include "rendercommands.h"
#include "../src/log.h"
#include "mesh.h"
#include "shader.h"
#include <glad/glad.h>
#include "../graphics/vertex.h"
#include <memory>
#include "../graphics/framebuffer.h"
#include "../src/engine.h"
#include "../graphics/texture.h"
namespace eclipse::graphics::rendercommands{
  void RenderVertexArray::Execute(){
    // weak ptr allows us to create a shared pointer
    std::shared_ptr<VertexArray>vertexArray = mVertexArray.lock();
    // if it has not been in use we get a nullpointer
    std::shared_ptr<shader> shader = mShader.lock();
    if(vertexArray && shader){
      vertexArray -> Bind();
      shader ->Bind();
      if(vertexArray -> GetElementCount()>0){
        glDrawElements(GL_TRIANGLES,vertexArray -> GetElementCount(),GL_UNSIGNED_INT,0);
      }

      else{
      glDrawArrays(GL_TRIANGLE_STRIP,0,vertexArray->GetVertexCount());
      }
      vertexArray->UnBind();
      shader ->UnBind();
    }else{
      ECLIPSE_WARN("Attempting to execute rendermesh with invalid data");
    }
  }

  void PushFrameBuffer::Execute(){
    std::shared_ptr<FrameBuffer>fb = mFrameBuffer.lock();
    if(fb){
      engine::Instance().GetRenderManager().PushFrameBuffer(fb);
    }else{
      ECLIPSE_WARN("Attempting to execute push frame buffer with invalid data");
      
    }
  }

  void PopFrameBuffer::Execute(){
      engine::Instance().GetRenderManager().PopFrameBuffer(nullptr);
  }

  void RenderVertexArrayTextured::Execute(){
    std::shared_ptr<VertexArray> vertexArray= mVertexArray.lock();
    std::shared_ptr<shader> shader = mShader.lock();
    std::shared_ptr<Texture>texture = mTexture.lock();
    if(vertexArray && shader && texture){
      vertexArray-> Bind();
      shader ->Bind();
      texture->Bind();
      if(vertexArray-> GetElementCount()>0){
        glDrawElements(GL_TRIANGLES,vertexArray -> GetElementCount(),GL_UNSIGNED_INT,0);
      }
      else{
      glDrawArrays(GL_TRIANGLE_STRIP,0,vertexArray->GetVertexCount());
      }
      vertexArray ->UnBind();
      texture->Unbind();
      shader ->UnBind();
    }else{
      ECLIPSE_WARN("Attempting to execute rendermesh with invalid data");
    }
  }
}
