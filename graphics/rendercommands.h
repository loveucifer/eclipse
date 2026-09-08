#pragma once
#include <memory>

namespace eclipse::graphics {
class Texture;
class mesh;
class shader;
class FrameBuffer;

namespace rendercommands {

class RenderCommand {
public:
  virtual void Execute() = 0;
  virtual ~RenderCommand() {}
};

class RenderMesh : public RenderCommand {
public:
  // queue for use
  RenderMesh(std::weak_ptr<mesh> mesh, std::weak_ptr<shader> shader)
      : mMesh(mesh), mShader(shader) {}


  virtual void Execute();

private:
  std::weak_ptr<mesh> mMesh;
  std::weak_ptr<shader> mShader;
};

class RenderMeshTextured : public RenderCommand {
public:
  // queue for use
  RenderMeshTextured(std::weak_ptr<mesh> mesh, std::weak_ptr<shader> shader,std::weak_ptr<Texture>texture)
      : mMesh(mesh), mShader(shader) ,mTexture(texture){}

  virtual void Execute();

private:
  std::weak_ptr<mesh> mMesh;
  std::weak_ptr<shader> mShader;
  std::weak_ptr<Texture>mTexture;
};



class PushFrameBuffer :public RenderCommand{
  public:
    PushFrameBuffer(std::weak_ptr<FrameBuffer>frameBuffer):mFrameBuffer(frameBuffer){}
    virtual void Execute() override ;
  private:
    std::weak_ptr<FrameBuffer>mFrameBuffer;
};

class PopFrameBuffer :public RenderCommand{
  public:
    PopFrameBuffer(){}
    virtual void Execute() override;
  private:
};

} // namespace rendercommands

} // namespace eclipse::graphics
