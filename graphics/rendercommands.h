#pragma once
#include <memory>
#include "../graphics/framebuffer.h"
namespace eclipse::graphics {
class Mesh;
class Shader;
class Texture;
namespace rendercommands {
class RenderCommands {
public:
  virtual void Execute() = 0;
  virtual ~RenderCommands() {}
};
class RenderMesh : public RenderCommands {
public:
  virtual void Execute();
  RenderMesh(std::weak_ptr<Mesh>mesh,std::weak_ptr<Shader>shader)
      : mMesh(mesh), mShader(shader) {}
private:
  std::weak_ptr<Mesh> mMesh;
  std::weak_ptr<Shader> mShader;
};


class RenderMeshTextured : public RenderCommands {
public:
  virtual void Execute();
  RenderMeshTextured(std::weak_ptr<Mesh> mesh,
                     std::weak_ptr<Shader> shader,
                     std::weak_ptr<Texture> texture,
                     const glm::mat4& model,
                     const glm::mat4& view,
                      const glm::mat4& projection,
                      const glm::vec2& uvOffset,
                      const glm::vec2& uvScale,
                      const glm::vec3& color = glm::vec3(1.0f))
      : mMesh(mesh), mShader(shader), mTexture(texture), mModel(model),
        mView(view), mProjection(projection), mUvOffset(uvOffset),
         mUvScale(uvScale), mColor(color) {}
private:
  std::weak_ptr<Mesh> mMesh;
  std::weak_ptr<Shader> mShader;
  std::weak_ptr<Texture>mTexture;
  glm::mat4 mModel;
  glm::mat4 mView;
  glm::mat4 mProjection;
  glm::vec2 mUvOffset;
  glm::vec2 mUvScale;
  glm::vec3 mColor;
};


class PushFrameBuffer: public RenderCommands{
  public:
    PushFrameBuffer(std::shared_ptr<graphics::FrameBuffer>frameBuffer):mFrameBuffer(frameBuffer){}
    void Execute() override;
  private:
    std::weak_ptr<FrameBuffer>mFrameBuffer;
};

class PopFrameBuffer : public RenderCommands{
  public:
    PopFrameBuffer(){}
    void Execute() override ;
  private:
};




} // namespace rendercommands
} // namespace eclipse::graphics
