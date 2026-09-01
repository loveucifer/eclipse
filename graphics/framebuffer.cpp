#include "framebuffer.h"
#include "helpers.h"
#include "../src/log.h"
#include <cstdint>

  namespace eclipse::graphics{
       FrameBuffer:: FrameBuffer(uint32_t width ,uint32_t height)
        :mFbo(0),
        mTextureId(0),
        mRenderBufferId(0),
        mCCR(1),mCCG(1),mCCB(1),mCCA(1),mWidth(width),mHeight(height)

       {
          glGenFramebuffers(1,&mFbo);ECLIPSE_CHECK_GL_ERROR;
          glBindFramebuffer(GL_FRAMEBUFFER,mFbo);ECLIPSE_CHECK_GL_ERROR;
          // use a texture for our framebufffer
          glGenTextures(1,&mTextureId);ECLIPSE_CHECK_GL_ERROR;
          glBindTexture(GL_TEXTURE_2D,mTextureId);ECLIPSE_CHECK_GL_ERROR;
          glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,mWidth,mHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);ECLIPSE_CHECK_GL_ERROR;
          // we dont have ant valid data at start;
          glBindTexture(GL_TEXTURE_2D,0);ECLIPSE_CHECK_GL_ERROR;
          glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,mTextureId,0);ECLIPSE_CHECK_GL_ERROR;


          // create depth and stencil renderbuffer

          glGenRenderbuffers(1,&mRenderBufferId);ECLIPSE_CHECK_GL_ERROR;
          glBindRenderbuffer(GL_RENDERBUFFER,mRenderBufferId);ECLIPSE_CHECK_GL_ERROR;
          glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8,mWidth,mHeight);ECLIPSE_CHECK_GL_ERROR;
          glBindRenderbuffer(GL_RENDERBUFFER,0);ECLIPSE_CHECK_GL_ERROR;
          glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,mRenderBufferId);ECLIPSE_CHECK_GL_ERROR;


          // check completeness

         int32_t completeStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);ECLIPSE_CHECK_GL_ERROR;
         if(completeStatus!=GL_FRAMEBUFFER_COMPLETE){
           ECLIPSE_ERROR("FrameBuffer creation failed : complete status {}",completeStatus);
         } 
          glBindFramebuffer(GL_FRAMEBUFFER,0);ECLIPSE_CHECK_GL_ERROR;
       }



       
      FrameBuffer::~FrameBuffer(){
       glDeleteFramebuffers(1,&mFbo);
       mFbo = 0;
      mTextureId = 0;
      mRenderBufferId= 0;
      }


      // window manages our framebuffer
    
  }
