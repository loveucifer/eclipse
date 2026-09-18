#include "vertex.h"
#include <cstdint>
#include <glad/glad.h>
#include <vector>
#include "helpers.h"
namespace eclipse::graphics{

const uint32_t RawVertexBuffer::GLTypeByte = GL_BYTE;
const uint32_t RawVertexBuffer::GLTypeUByte = GL_UNSIGNED_BYTE;
const uint32_t RawVertexBuffer::GLTypeShort = GL_SHORT;
const uint32_t RawVertexBuffer::GLTypeUShort = GL_UNSIGNED_SHORT;
const uint32_t RawVertexBuffer::GLTypeInt = GL_INT;
const uint32_t RawVertexBuffer::GLTypeUInt = GL_UNSIGNED_INT;
const uint32_t RawVertexBuffer::GLTypeFloat = GL_FLOAT;
const uint32_t RawVertexBuffer::GLTypeDouble = GL_DOUBLE;

RawVertexBuffer::RawVertexBuffer() {
  glGenBuffers(1, &mVbo);
  ECLIPSE_CHECK_GL_ERROR;
}
  RawVertexBuffer::~RawVertexBuffer(){
    glDeleteBuffers(1,&mVbo);ECLIPSE_CHECK_GL_ERROR;
  }
  void RawVertexBuffer::SetLayout(const std::vector<uint32_t>& layout){
    mLayout = layout;
    mStride = 0;
    for(auto& count : layout){
      mStride += count; // no o float in the row ( this is the raw vertex buffer)
      // so we keep stride as no of values per rows
    }
  }
  // boo dynamic is set to false as its a flag for our GL_STATIC_DRAW call to make the
  // distinction between whether its a dynamic or static call
  void RawVertexBuffer::Upload(bool dynamic) {
    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    ECLIPSE_CHECK_GL_ERROR;
    glBufferData(
        GL_ARRAY_BUFFER,
        mSize,
        mData,
        dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW
    );

    ECLIPSE_CHECK_GL_ERROR;
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    ECLIPSE_CHECK_GL_ERROR;
    mIsUploaded = true;
  }

  void RawVertexBuffer::Bind(){
    glBindBuffer(GL_ARRAY_BUFFER, mVbo);

  }

  void RawVertexBuffer::Unbind(){
    glBindBuffer(GL_ARRAY_BUFFER, 0);

  }

  VertexArray::VertexArray()
  :mVao(0),mEbo(0),mAttributeCount(0),mVertexCount(0),mElementCount(0),mIsValid(false)
  {
        glGenVertexArrays(1,&mVao);ECLIPSE_CHECK_GL_ERROR;
  }
  VertexArray::~VertexArray(){

        uint32_t id;

        for (auto& vbo: mVbos){
          id = vbo->GetVbo();
          glDeleteBuffers(1,&id);ECLIPSE_CHECK_GL_ERROR;
          delete vbo;
        }
        glDeleteVertexArrays(1,&mVao);ECLIPSE_CHECK_GL_ERROR;
        mVbos.clear();
  }
  void VertexArray::PushBuffer(RawVertexBuffer *Vbo) {
    if(mVbos.size()>0){
      ECLIPSE_ASSERT(mVbos[0]->GetVertexCount() == Vbo->GetVertexCount() ,"VertexBuffer::PushBuffer - Attempting to push a vertexbuffer with a different vertex count");
    }
    ECLIPSE_ASSERT(Vbo->GetLayout().size() > 0 , "VertexBuffer::Pushbuffer - vertexbuffer has no layout defined");
    if(Vbo->GetLayout().size()>0){

    mVbos.push_back(Vbo);
    mVertexCount = (uint32_t)mVbos[0]->GetVertexCount();
    }
  }
  void VertexArray::SetElements(const std::vector<uint32_t> &elements){
    mElementCount = (uint32_t)elements.size();
    glBindVertexArray(mVao);ECLIPSE_CHECK_GL_ERROR;
    glGenBuffers(1,&mEbo);ECLIPSE_CHECK_GL_ERROR;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mEbo);ECLIPSE_CHECK_GL_ERROR;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,elements.size()*sizeof(uint32_t),&elements[0],GL_STATIC_DRAW);ECLIPSE_CHECK_GL_ERROR;
    glBindVertexArray(0);ECLIPSE_CHECK_GL_ERROR;

  }
  void VertexArray::Upload(){
    Bind();
    uint32_t attributeCount = 0;
    for(auto& vbo: mVbos){
      if(!vbo->GetIsUploaded()){
        vbo->Upload(false);
      }
      vbo->Bind();
      uint32_t offset = 0;
      for(uint32_t count : vbo->GetLayout()){
        glEnableVertexAttribArray(attributeCount);
        glVertexAttribPointer(attributeCount,count ,static_cast<GLenum>(vbo->GetGLType()),GL_FALSE,vbo->GetStride(),(void*)(intptr_t)offset);
        attributeCount++;
        offset+=(count * vbo->GetTypeSize());

      }
      vbo->Unbind();
    }

    glBindVertexArray(0);
    mAttributeCount = attributeCount;
    mIsValid = mVertexCount > 0 && (mElementCount > 0 || attributeCount > 0);
  }
  void VertexArray::Bind(){
    glBindVertexArray(mVao);
  }
  void VertexArray::UnBind(){

    glBindVertexArray(0);
  }
}
