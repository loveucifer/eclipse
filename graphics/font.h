#pragma once

#include "mesh.h"
#include "texture.h"

#include <array>
#include <cstdint>
#include <memory>
#include <string>

#include <glm/glm.hpp>

namespace eclipse::graphics {

struct Glyph {
  glm::vec2 size{0.0f};
  glm::vec2 bearing{0.0f};
  glm::vec2 uvOffset{0.0f};
  glm::vec2 uvScale{0.0f};
  float advance = 0.0f;
};

class Font {
public:
  Font(const std::string& path, std::uint32_t pixelHeight);

  bool IsLoaded() const { return mLoaded; }
  std::uint32_t GetPixelHeight() const { return mPixelHeight; }
  float GetLineHeight() const { return mLineHeight; }
  const Glyph& GetGlyph(unsigned char character) const;
  const std::shared_ptr<Texture>& GetAtlas() const { return mAtlas; }
  const std::shared_ptr<Mesh>& GetMesh() const { return mMesh; }

private:
  static constexpr unsigned char FirstCharacter = 32;
  static constexpr unsigned char LastCharacter = 126;

  std::array<Glyph, 128> mGlyphs{};
  std::shared_ptr<Texture> mAtlas;
  std::shared_ptr<Mesh> mMesh;
  std::uint32_t mPixelHeight = 0;
  float mLineHeight = 0.0f;
  bool mLoaded = false;
};

} // namespace eclipse::graphics
