#pragma once
#include <memory>

namespace eclipse::graphics {
class Texture;
class VertexArray;
class shader;
class FrameBuffer;

namespace rendercommands {

class RenderCommand {
public:
  virtual void Execute() = 0;
  virtual ~RenderCommand() {}
};

class RenderVertexArray: public RenderCommand {
public:
  // queue for use
  RenderVertexArray(std::weak_ptr<VertexArray> vertexArray, std::weak_ptr<shader> shader)
      : mVertexArray(vertexArray), mShader(shader) {}


  virtual void Execute();

private:
  std::weak_ptr<VertexArray> mVertexArray;
  std::weak_ptr<shader> mShader;
};

class RenderVertexArrayTextured : public RenderCommand {
public:
  // queue for use
  RenderVertexArrayTextured(
      std::weak_ptr<VertexArray> vertexArray,
      std::weak_ptr<shader> shader,
      std::weak_ptr<Texture> texture
  )
      : mVertexArray(vertexArray), mShader(shader), mTexture(texture) {}

  virtual void Execute();

private:
  std::weak_ptr<VertexArray> mVertexArray;
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
