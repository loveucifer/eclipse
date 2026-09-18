#pragma once
#include <cstdint>
#include "../src/log.h"
#include <vector>
#include <type_traits>
namespace eclipse::graphics{
  class RawVertexBuffer{

    public:
      static const uint32_t GLTypeByte;
      static const uint32_t GLTypeUByte;
      static const uint32_t GLTypeShort;
      static const uint32_t GLTypeUShort;
      static const uint32_t GLTypeInt;
      static const uint32_t GLTypeUInt;
      static const uint32_t GLTypeFloat;
      static const uint32_t GLTypeDouble;

    public:
      RawVertexBuffer();
      virtual ~RawVertexBuffer();
      inline uint32_t GetVertexCount(){return mVertexCount;}

      virtual uint32_t GetTypeSize() const  = 0;


      inline uint32_t GetVbo(){return mVbo;}
      inline uint32_t GetSize(){return mSize;}
      inline uint32_t GetStride(){return mStride;}
      inline  const std::vector<uint32_t>& GetLayout() const {return mLayout;}
      inline uint32_t GetGLType() const {return mGLType;}
      void SetLayout(const std::vector<uint32_t>& layout);
      virtual void Upload(bool dynamic = false);
      void Bind();
      void Unbind();
      inline bool GetIsUploaded()const {return mIsUploaded;}

      protected:

        bool mIsUploaded = false;
        uint32_t mGLType = 0;
        uint32_t mVbo = 0;
        uint32_t mVertexCount = 0;
        uint32_t mStride= 0;
        std::vector<uint32_t>mLayout;
        void* mData = nullptr;
        uint32_t mSize = 0;
  };

  template <typename T>

  class VertexBuffer : public RawVertexBuffer {
    static_assert(
        std::is_same<T, char>() ||
        std::is_same<T, unsigned char>() ||
        std::is_same<T, short>() ||
        std::is_same<T, unsigned short>() ||
        std::is_same<T, int>() ||
        std::is_same<T, unsigned int>() ||
        std::is_same<T, float>() ||
        std::is_same<T, double>(),
    "This type is not supported"

    );

  public:
    VertexBuffer() {

      if constexpr (std::is_same<T, char>()) {
        mGLType = RawVertexBuffer::GLTypeByte;
      }
      if constexpr (std::is_same<T, unsigned char>()) {
        mGLType = RawVertexBuffer::GLTypeUByte;
      }
      if constexpr (std::is_same<T, short>()) {
        mGLType = RawVertexBuffer::GLTypeShort;
      }
      if constexpr (std::is_same<T, unsigned short>()) {
        mGLType = RawVertexBuffer::GLTypeUShort;
      }
      if constexpr (std::is_same<T, int>()) {
        mGLType = RawVertexBuffer::GLTypeInt;
      }
      if constexpr (std::is_same<T, unsigned int>()) {
        mGLType = RawVertexBuffer::GLTypeUInt;
      }
      if constexpr (std::is_same<T, float>()) {
        mGLType = RawVertexBuffer::GLTypeFloat;
      }
      if constexpr (std::is_same<T, double>()) {
        mGLType = RawVertexBuffer::GLTypeDouble;
      }
    }
    ~VertexBuffer() {}
    uint32_t GetTypeSize() const override { return sizeof(T); }

    void PushVertex(const std::vector<T> &vertex) {
      mVertexCount++;
      mDataVector.insert(mDataVector.end(), vertex.begin(), vertex.end());
    }
    void Upload(bool dynamic = false) override {
      mStride *= sizeof(T);
      mSize = sizeof(T) * (uint32_t)mDataVector.size();
      ECLIPSE_TRACE(
          "VertexBuffer::Upload() -mSize: {} -mStride: {}",
          mSize,
          mStride
      );
      ECLIPSE_ASSERT(mSize > 0, "Trying to upload Vbo that has no data ");
      mData = &mDataVector[0];
      RawVertexBuffer::Upload(dynamic);
    }

  private:
    // auto figure out whatever type float or int or whatver we have to deal
    // with
    std::vector<T> mDataVector;
  };

    class VertexArray{
      public:
        VertexArray();
        ~VertexArray();
        inline bool IsValid() const {return mIsValid;}
        inline uint32_t GetVertexCount(){return mVertexCount;}
        inline uint32_t GetElementCount(){return mElementCount;}
        inline uint32_t GetAttributeCount(){return mAttributeCount;}
        void PushBuffer(RawVertexBuffer* Vbo);
        void SetElements(const std::vector<uint32_t>& elements);
        void Upload();
        void Bind();
        void UnBind();
      private:
        bool mIsValid;

        uint32_t mVertexCount , mElementCount;
        uint32_t mVao,mEbo;
        uint32_t mAttributeCount;
        std::vector<RawVertexBuffer*>mVbos;
    };









}
