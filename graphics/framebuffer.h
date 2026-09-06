#pragma once
#include <cstdint>
#include "../glm/glm.hpp"
namespace eclipse::graphics{
  class FrameBuffer{
    public:
      FrameBuffer(uint32_t width ,uint32_t height);
      ~FrameBuffer();
      inline uint32_t GetFbo()const {return mFbo;}
      inline uint32_t GetRenderBufferId()const {return mRenderBufferId;}
      inline uint32_t GetTextureId()const {return mTextureId;}
      inline const glm::ivec2& GetSize(){return mSize;}
      inline void SetClearColor(const glm::vec4& clearColor){mClearColor = clearColor;}
      inline const glm::vec4& GetClearColor(){return mClearColor;}
    private:
      uint32_t mFbo;
      uint32_t mTextureId;
      uint32_t mRenderBufferId;
      glm::ivec2 mSize;
      glm::vec4 mClearColor;
  };
};
