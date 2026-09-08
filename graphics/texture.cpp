#include "texture.h"
#include <cstdint>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"
#include <glad/glad.h>
#include "../src/log.h"
#include "../graphics/helpers.h"
namespace eclipse::graphics{


void Texture::SetTextureFilter(TextureFilter filter){

  mFilter = filter;
  glBindTexture(GL_TEXTURE_2D, mId);
  ECLIPSE_CHECK_GL_ERROR;
  switch(mFilter){
    case TextureFilter::Linear:
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      break;
     case TextureFilter::Nearest:
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      break;
    }
    glBindTexture(GL_TEXTURE_2D,0);
}

Texture::Texture(const std::string &path)
    : mFilter(TextureFilter::Linear), mPath(path), mWidth(0), mHeight(0),
      mNumChannels(0), mPixels(nullptr) {

  int width, height, numChannels;
  stbi_set_flip_vertically_on_load(1);
  mPixels = stbi_load(path.c_str(), &width, &height, &numChannels, 0);
  if (mPixels) {
    mWidth = (uint32_t)width;
    mHeight = (uint32_t)height;
    mNumChannels = (uint32_t)numChannels;
  }
  LoadTexture();
}

  Texture::~Texture(){
    stbi_image_free(mPixels);
    mPixels = nullptr;
  }

  void Texture::LoadTexture(){
    glGenTextures(1,&mId);ECLIPSE_CHECK_GL_ERROR;
    glBindTexture(GL_TEXTURE_2D,mId);ECLIPSE_CHECK_GL_ERROR;

      GLenum dataFormat = 0;
      if(mNumChannels == 4){
        dataFormat = GL_RGBA;
      }else if(mNumChannels == 3){
        dataFormat = GL_RGB;
      }
      if(mPixels && dataFormat == 0){

        ECLIPSE_ERROR("Texture format not supported - no of channels = {}",mNumChannels);
      }

     if(mPixels && dataFormat != 0 ){
       glTexImage2D(
           GL_TEXTURE_2D,
           0,
           dataFormat,
           mWidth,
           mHeight,
           0,
           dataFormat,
           GL_UNSIGNED_BYTE,
           mPixels
       );ECLIPSE_CHECK_GL_ERROR;

        SetTextureFilter(mFilter);
       ECLIPSE_TRACE("loaded {}channel texture{}", mNumChannels, mPath.c_str());
    }else{
      float pixels[] = {
        1.f,0.f,1.f,     1.f,1.f,1.f,   1.f,0.f,1.f,      1.f,1.f,1.f,

       1.f,1.f,1.f,   1.f,0.f,1.f, 1.f,1.f,1.f, 1.f,0.f,1.f,

        1.f,0.f,1.f,     1.f,1.f,1.f,   1.f,0.f,1.f,      1.f,1.f,1.f,

       1.f,1.f,1.f,   1.f,0.f,1.f, 1.f,1.f,1.f, 1.f,0.f,1.f,

      };
      mWidth = 4;
      mHeight = 4;
      mNumChannels = 3;

       glTexImage2D(
           GL_TEXTURE_2D,
           0,
           GL_RGB,
           mWidth,
           mHeight,
           0,
           GL_RGB,
           GL_FLOAT,
           pixels);ECLIPSE_CHECK_GL_ERROR;
           SetTextureFilter(TextureFilter::Nearest);
           ECLIPSE_WARN("Unable to load texture{}: {} ,defaulting to checkboard",  mPath.c_str());
    }

    glBindTexture(GL_TEXTURE_2D,0);

  }
  void Texture::Bind(){
    glBindTexture(GL_TEXTURE_2D,mId);
  }
  void Texture::Unbind(){
    glBindTexture(GL_TEXTURE_2D,0);
  }
}
