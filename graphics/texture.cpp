#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#include "../src/log.h"

#include <glad/glad.h>

namespace eclipse::graphics {

Texture::Texture(const std::string& path, TextureFilter filter)
    : mPath(path), mFilter(filter) {
  int width = 0;
  int height = 0;
  int channels = 0;
  stbi_set_flip_vertically_on_load(true);
  mPixels = stbi_load(path.c_str(), &width, &height, &channels, 0);
  if (mPixels) {
    mWidth = static_cast<std::uint32_t>(width);
    mHeight = static_cast<std::uint32_t>(height);
    mNumChannels = static_cast<std::uint32_t>(channels);
  }
  LoadTexture();
  stbi_image_free(mPixels);
  mPixels = nullptr;
}

Texture::Texture(std::uint32_t width, std::uint32_t height,
                 std::uint32_t channels, const unsigned char* pixels,
                 TextureFilter filter, bool alphaMask)
    : mFilter(filter), mWidth(width), mHeight(height),
      mNumChannels(channels), mPixels(const_cast<unsigned char*>(pixels)),
      mAlphaMask(alphaMask) {
  LoadTexture();
  mPixels = nullptr;
}

Texture::~Texture() {
  if (mId != 0) {
    glDeleteTextures(1, &mId);
  }
}

void Texture::LoadTexture() {
  glGenTextures(1, &mId);
  glBindTexture(GL_TEXTURE_2D, mId);

  GLenum dataFormat = 0;
  if (mNumChannels == 4) {
    dataFormat = GL_RGBA;
  } else if (mNumChannels == 3) {
    dataFormat = GL_RGB;
  } else if (mNumChannels == 1) {
    dataFormat = GL_RED;
  }

  if (mPixels && dataFormat != 0) {
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(dataFormat),
                 static_cast<GLsizei>(mWidth), static_cast<GLsizei>(mHeight), 0,
                 dataFormat, GL_UNSIGNED_BYTE, mPixels);
    mLoadedFromSource = true;
    if (mAlphaMask) {
      constexpr GLint swizzle[] = {GL_ONE, GL_ONE, GL_ONE, GL_RED};
      glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
    }
  } else {
    constexpr unsigned char pixels[] = {
        255, 0,   255, 255, 255, 255, 255, 0,   255, 255, 255, 255,
        255, 255, 255, 255, 0,   255, 255, 255, 255, 255, 0,   255,
        255, 0,   255, 255, 255, 255, 255, 0,   255, 255, 255, 255,
        255, 255, 255, 255, 0,   255, 255, 255, 255, 255, 0,   255,
    };
    mWidth = 4;
    mHeight = 4;
    mNumChannels = 3;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<GLsizei>(mWidth),
                 static_cast<GLsizei>(mHeight), 0, GL_RGB, GL_UNSIGNED_BYTE,
                 pixels);
    mFilter = TextureFilter::Nearest;
    ECLIPSE_WARN("Unable to load texture '{}', defaulting to checkerboard",
                 mPath);
  }

  ApplyTextureFilter();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::ApplyTextureFilter() const {
  const GLint filter =
      mFilter == TextureFilter::Nearest ? GL_NEAREST : GL_LINEAR;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
}

void Texture::Bind() const { glBindTexture(GL_TEXTURE_2D, mId); }

void Texture::UnBind() const { glBindTexture(GL_TEXTURE_2D, 0); }

} // namespace eclipse::graphics
