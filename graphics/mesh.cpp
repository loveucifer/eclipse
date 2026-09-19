#include "mesh.h"
#include "helpers.h"
#include <cstdint>
#include <glad/glad.h>

namespace eclipse::graphics {

Mesh::Mesh(float *vertexArray, uint32_t vertexCount, uint32_t dimensions)
    : mVertexCount(vertexCount), mElementCount(0), mVao(0), mEbo(0),
      mPositionVbo(0) {
  glGenVertexArrays(1, &mVao);
  ECLIPSE_CHECK_GL_ERROR;
  glBindVertexArray(mVao);
  ECLIPSE_CHECK_GL_ERROR;
  glGenBuffers(1, &mPositionVbo);
  ECLIPSE_CHECK_GL_ERROR;
  glBindBuffer(GL_ARRAY_BUFFER, mPositionVbo);
  ECLIPSE_CHECK_GL_ERROR;
  glBufferData(
      GL_ARRAY_BUFFER,
      sizeof(float) * dimensions * vertexCount,
      vertexArray,
      GL_STATIC_DRAW
  );
  ECLIPSE_CHECK_GL_ERROR;
  glEnableVertexAttribArray(0);
  ECLIPSE_CHECK_GL_ERROR;
  glVertexAttribPointer(0, dimensions, GL_FLOAT, GL_FALSE, 0, 0);
  ECLIPSE_CHECK_GL_ERROR;
  glDisableVertexAttribArray(0);
  ECLIPSE_CHECK_GL_ERROR;
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  ECLIPSE_CHECK_GL_ERROR;
  glBindVertexArray(0);
  ECLIPSE_CHECK_GL_ERROR;
}
Mesh::Mesh(
    float *vertexArray,
    uint32_t vertexCount,
    uint32_t dimensions,
    uint32_t *elementArray,
    uint32_t elementCount
)
    : Mesh(vertexArray, vertexCount, dimensions) {
  mElementCount = elementCount;
  // glGenVertexArrays(1,&mVao);
  glBindVertexArray(mVao);
  ECLIPSE_CHECK_GL_ERROR;
  glGenBuffers(1, &mEbo);
  ECLIPSE_CHECK_GL_ERROR;
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEbo);
  ECLIPSE_CHECK_GL_ERROR;
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      sizeof(uint32_t) * elementCount,
      elementArray,
      GL_STATIC_DRAW
  );
  ECLIPSE_CHECK_GL_ERROR;
  // glEnableVertexAttribArray(0);
  // glVertexAttribPointer(0,dimensions,GL_FLOAT,GL_FALSE,0,0);
  // glDisableVertexAttribArray(0);
  // glBindBuffer(GL_ARRAY_BUFFER,0);
  glBindVertexArray(mVao);
  ECLIPSE_CHECK_GL_ERROR;
}

Mesh::~Mesh() {
  glDeleteVertexArrays(1, &mVao);
  ECLIPSE_CHECK_GL_ERROR;
  if (mEbo != 0) {
    glDeleteBuffers(1, &mEbo);
    ECLIPSE_CHECK_GL_ERROR;
  }
  glDeleteBuffers(1, &mPositionVbo);
  if (mTexCordsVbo != 0) glDeleteBuffers(1, &mTexCordsVbo);
  ECLIPSE_CHECK_GL_ERROR;
}

void Mesh::Bind() {
  glBindVertexArray(mVao);
  ECLIPSE_CHECK_GL_ERROR;
  glEnableVertexAttribArray(0);
  ECLIPSE_CHECK_GL_ERROR;
}

void Mesh::Unbind() {
  glDisableVertexAttribArray(0);
  ECLIPSE_CHECK_GL_ERROR;
  glBindVertexArray(0);
  ECLIPSE_CHECK_GL_ERROR;
}


  Mesh::Mesh(
      float *vertexArray,
      uint32_t vertexCount,
      uint32_t dimensions,
      float* texCoords,
      uint32_t *elementArray,
      uint32_t elementCount
                                 
  ):Mesh(vertexArray,vertexCount,dimensions,elementArray,elementCount)


  {
    
  // glGenVertexArrays(1, &mVao);
  // ECLIPSE_CHECK_GL_ERROR;
  glBindVertexArray(mVao);
  ECLIPSE_CHECK_GL_ERROR;
  glGenBuffers(1, &mTexCordsVbo);
  ECLIPSE_CHECK_GL_ERROR;
  glBindBuffer(GL_ARRAY_BUFFER, mTexCordsVbo);
  ECLIPSE_CHECK_GL_ERROR;
  glBufferData(
      GL_ARRAY_BUFFER,
      sizeof(float) * 2 * vertexCount,
      texCoords,
      GL_STATIC_DRAW
  );
  ECLIPSE_CHECK_GL_ERROR;
  glEnableVertexAttribArray(1);
  ECLIPSE_CHECK_GL_ERROR;
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
  ECLIPSE_CHECK_GL_ERROR;
  glDisableVertexAttribArray(0);
  ECLIPSE_CHECK_GL_ERROR;
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  ECLIPSE_CHECK_GL_ERROR;
  glBindVertexArray(0);
  ECLIPSE_CHECK_GL_ERROR;
  }



} // namespace eclipse::graphics
