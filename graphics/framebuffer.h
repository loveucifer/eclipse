#pragma once
#include <cstdint>
namespace eclipse::graphics{
  class FrameBuffer{
    public:
      FrameBuffer(uint32_t width ,uint32_t height);
      ~FrameBuffer();
      inline uint32_t GetFbo()const {return mFbo;}
      inline uint32_t GetRenderBufferId()const {return mRenderBufferId;}
      inline uint32_t GetTextureId()const {return mTextureId;}
      inline void GetSize(uint32_t& w, uint32_t& h){w = mWidth; h =mHeight;}
      inline void SetClearColor(float r, float g , float b , float a){mCCR = r,mCCG = g,mCCB = b,mCCA = a;}
      inline void GetClearColor(float& r, float& g , float& b , float& a){r =mCCR, g =mCCG,b=mCCB,a=mCCA;}
    private:
      uint32_t mFbo;
      uint32_t mTextureId;
      uint32_t mRenderBufferId;

      uint32_t mWidth, mHeight;
      float mCCR, mCCG, mCCB,mCCA;
  };
};
