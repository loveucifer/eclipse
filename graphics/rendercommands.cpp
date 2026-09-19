#include "rendercommands.h"
#include "../src/engine.h"
#include "../src/log.h"
#include <glad/glad.h>
#include <memory>
#include "texture.h"
#include "mesh.h"
#include "shader.h"
#include "../graphics/helpers.h"
namespace eclipse::graphics::rendercommands{
  void RenderMesh::Execute(){
    std::shared_ptr<Mesh>mesh = mMesh.lock();
    std::shared_ptr<Shader>shader = mShader.lock();
    if( mesh && shader){
      mesh ->Bind();
      shader ->Bind();
      if(mesh ->GetElementCount() >0){
        glDrawElements(GL_TRIANGLES,mesh->GetElementCount(),GL_UNSIGNED_INT,0);
        ECLIPSE_CHECK_GL_ERROR;
      }else{
        glDrawArrays(GL_TRIANGLE_STRIP,0,mesh->GetVertexCount());
        ECLIPSE_CHECK_GL_ERROR;
      }
      mesh ->Unbind();
      shader ->Unbind();
    }else{
      ECLIPSE_WARN("Initializing with invalid shader data");
    }
  }

  
  void RenderMeshTextured::Execute(){
    std::shared_ptr<Mesh>mesh = mMesh.lock();
    std::shared_ptr<Texture>texture = mTexture.lock();
    std::shared_ptr<Shader>shader = mShader.lock();
    if( mesh && shader && texture){
      mesh ->Bind();
      texture ->Bind();
      shader ->Bind();
      shader->SetUniformMat4("model", mModel);
      shader->SetUniformMat4("view", mView);
      shader->SetUniformMat4("projection", mProjection);
      shader->SetUniformFloat2("uvOffset", mUvOffset);
      shader->SetUniformFloat2("uvScale", mUvScale);
      shader->SetUniformFloat3("color", mColor);
      if(mesh ->GetElementCount() >0){
        glDrawElements(GL_TRIANGLES,mesh->GetElementCount(),GL_UNSIGNED_INT,0);
        ECLIPSE_CHECK_GL_ERROR;
      }else{
        glDrawArrays(GL_TRIANGLE_STRIP,0,mesh->GetVertexCount());
        ECLIPSE_CHECK_GL_ERROR;
      }
      mesh ->Unbind();
      texture->UnBind();
      shader ->Unbind();
    }else{
      ECLIPSE_WARN("Initializing with invalid shader data");
    }
  }

  void PushFrameBuffer::Execute(){
    std::shared_ptr<FrameBuffer>fb= mFrameBuffer.lock();
    if(fb){
      Engine::Instance().GetRenderManager().PushFrameBuffer(fb);
    }else{
      ECLIPSE_WARN("Stack empty");
    }
  }

  void PopFrameBuffer::Execute(){
    Engine::Instance().GetRenderManager().PopFrameBuffer(nullptr);
  }


  
}
