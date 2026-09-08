#include "rendercommands.h"
#include "../src/log.h"
#include "mesh.h"
#include "shader.h"
#include <glad/glad.h>
#include <memory>
#include "../graphics/framebuffer.h"
#include "../src/engine.h"
#include "../graphics/texture.h"
namespace eclipse::graphics::rendercommands{
  void RenderMesh::Execute(){
    // weak ptr allows us to create a shared pointer
    std::shared_ptr<mesh> mesh = mMesh.lock();
    // if it has not been in use we get a nullpointer
    std::shared_ptr<shader> shader = mShader.lock();
    if(mesh&& shader){
      mesh -> Bind();
      shader ->Bind();
      if(mesh -> GetElementCount()>0){
        glDrawElements(GL_TRIANGLES,mesh -> GetElementCount(),GL_UNSIGNED_INT,0);
      }

      else{
      glDrawArrays(GL_TRIANGLE_STRIP,0,mesh->GetVertexCount());
      }
      mesh ->UnBind();
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

  void RenderMeshTextured::Execute(){
    std::shared_ptr<mesh> mesh = mMesh.lock();
    std::shared_ptr<shader> shader = mShader.lock();
    std::shared_ptr<Texture>texture = mTexture.lock();
    if(mesh&& shader && texture){
      mesh -> Bind();
      shader ->Bind();
      texture->Bind();
      if(mesh -> GetElementCount()>0){
        glDrawElements(GL_TRIANGLES,mesh -> GetElementCount(),GL_UNSIGNED_INT,0);
      }
      else{
      glDrawArrays(GL_TRIANGLE_STRIP,0,mesh->GetVertexCount());
      }
      mesh ->UnBind();
      texture->Unbind();
      shader ->UnBind();
    }else{
      ECLIPSE_WARN("Attempting to execute rendermesh with invalid data");
    }
  }
}
