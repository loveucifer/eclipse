#pragma once
#include <cstdint>
namespace eclipse::graphics {
class Mesh {
public:
  Mesh(float *vertexArray, uint32_t vertexCount, uint32_t dimensions);
  Mesh(
      float *vertexArray,
      uint32_t vertexCount,
      uint32_t dimensions,
      uint32_t *elementArray,
      uint32_t elementCount
  );

  Mesh(
      float *vertexArray,
      uint32_t vertexCount,
      uint32_t dimensions,
      float* texCoords,
      uint32_t *elementArray,
      uint32_t elementCount
                                 
  );



  inline uint32_t GetElementCount(){return mElementCount;}
  inline uint32_t GetVertexCount(){return mVertexCount;}
  ~Mesh();
  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;
  Mesh(Mesh&&) = delete;
  Mesh& operator=(Mesh&&) = delete;
  void Bind();
  void Unbind();

private:
  uint32_t mVertexCount , mElementCount;
  uint32_t mVao,mEbo;
  uint32_t mPositionVbo;
  uint32_t mTexCordsVbo = 0;

};
} // namespace eclipse::graphics
