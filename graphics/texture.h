#pragma once
#include <cstdint>
#include <string>
namespace eclipse::graphics{


  enum class TextureFilter{
    Nearest,
    Linear
  };

  class Texture{
    public:
      Texture(const std::string& path);
      ~Texture();
      inline uint32_t GetId(){return mId;}
      inline uint32_t GetWidth(){return mWidth;}
      inline uint32_t GetHeight(){return mHeight;}
      inline uint32_t GetNumChannels(){return mNumChannels;}
      inline const std::string& GetPath(){return mPath;}
      inline TextureFilter GetTextureFilter(){return mFilter;}
      void Bind();
      void Unbind();
      void SetTextureFilter(TextureFilter filter);

    private:
      void LoadTexture();



    private:

      TextureFilter mFilter;

      std::string mPath;
      uint32_t mId;
      uint32_t mWidth,mHeight;
      uint32_t mNumChannels;
      unsigned char* mPixels;
  };
}
