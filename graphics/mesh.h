#pragma once
#include <cstdint>
#include <iostream>

namespace eclipse::graphics{
  class mesh{
    public:
      mesh(float* vertexArray, uint32_t vertexCount, uint32_t dimensions);

      mesh(float* vertexArray, uint32_t vertexCount, uint32_t dimensions ,uint32_t *elementArray, uint32_t elementCount);
      ~mesh();

      void Bind();
      void UnBind();

      inline uint32_t  GetVertexCount() const {return mVertexCount;}
      inline uint32_t  GetElementCount() const {return mElementCount;}

    private:

      uint32_t mElementCount;
      uint32_t mVertexCount;

      uint32_t mVao;
      uint32_t mEbo;
      uint32_t mPositionVbo;
  };
}
