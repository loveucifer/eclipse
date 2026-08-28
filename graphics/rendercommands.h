#pragma once
#include <memory>

namespace eclipse::graphics {

class mesh;
class shader;

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

} // namespace rendercommands

} // namespace eclipse::graphics
