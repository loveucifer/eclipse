#pragma once

#include <cstdint>
#include <string>

namespace eclipse::graphics {

enum class TextureFilter {
  Nearest,
  Linear,
};

class Texture {
public:
  explicit Texture(const std::string& path,
                   TextureFilter filter = TextureFilter::Linear);
  Texture(std::uint32_t width, std::uint32_t height,
          std::uint32_t channels, const unsigned char* pixels,
          TextureFilter filter = TextureFilter::Linear,
          bool alphaMask = false);
  ~Texture();

  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;
  Texture(Texture&&) = delete;
  Texture& operator=(Texture&&) = delete;

  const std::string& GetPath() const { return mPath; }
  std::uint32_t GetWidth() const { return mWidth; }
  std::uint32_t GetHeight() const { return mHeight; }
  std::uint32_t GetId() const { return mId; }
  std::uint32_t GetNumChannels() const { return mNumChannels; }
  TextureFilter GetTextureFilter() const { return mFilter; }
  bool IsLoadedFromSource() const { return mLoadedFromSource; }

  void Bind() const;
  void UnBind() const;

private:
  void LoadTexture();
  void ApplyTextureFilter() const;

  std::string mPath;
  TextureFilter mFilter;
  std::uint32_t mId = 0;
  std::uint32_t mWidth = 0;
  std::uint32_t mHeight = 0;
  std::uint32_t mNumChannels = 0;
  unsigned char* mPixels = nullptr;
  bool mLoadedFromSource = false;
  bool mAlphaMask = false;
};

} // namespace eclipse::graphics
