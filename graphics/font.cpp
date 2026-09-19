#include "font.h"

#include "../src/log.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <vector>

namespace eclipse::graphics {

Font::Font(const std::string& path, std::uint32_t pixelHeight)
    : mPixelHeight(pixelHeight) {
  if (pixelHeight == 0) {
    ECLIPSE_ERROR("Font pixel height must be greater than zero");
    return;
  }

  FT_Library library = nullptr;
  if (FT_Init_FreeType(&library) != 0) {
    ECLIPSE_ERROR("Unable to initialize FreeType");
    return;
  }

  FT_Face face = nullptr;
  if (FT_New_Face(library, path.c_str(), 0, &face) != 0) {
    ECLIPSE_ERROR("Unable to load font '{}'", path);
    FT_Done_FreeType(library);
    return;
  }

  if (FT_Set_Pixel_Sizes(face, 0, pixelHeight) != 0) {
    ECLIPSE_ERROR("Unable to set font size for '{}'", path);
    FT_Done_Face(face);
    FT_Done_FreeType(library);
    return;
  }

  constexpr std::uint32_t columns = 16;
  constexpr std::uint32_t rows = 6;
  const std::uint32_t cellSize = pixelHeight + 2;
  const std::uint32_t atlasWidth = columns * cellSize;
  const std::uint32_t atlasHeight = rows * cellSize;
  std::vector<unsigned char> pixels(atlasWidth * atlasHeight, 0);

  bool valid = true;
  for (unsigned int code = FirstCharacter; code <= LastCharacter; ++code) {
    if (FT_Load_Char(face, code, FT_LOAD_RENDER) != 0) {
      ECLIPSE_WARN("Unable to rasterize character {} from '{}'", code, path);
      valid = false;
      continue;
    }

    const FT_GlyphSlot slot = face->glyph;
    const std::uint32_t index = code - FirstCharacter;
    const std::uint32_t cellX = (index % columns) * cellSize;
    const std::uint32_t cellY = (index / columns) * cellSize;
    const std::uint32_t width = slot->bitmap.width;
    const std::uint32_t height = slot->bitmap.rows;
    if (width > cellSize || height > cellSize) {
      ECLIPSE_ERROR("Character {} is too large for the font atlas", code);
      valid = false;
      continue;
    }

    for (std::uint32_t y = 0; y < height; ++y) {
      for (std::uint32_t x = 0; x < width; ++x) {
        const auto source = y * static_cast<std::uint32_t>(slot->bitmap.pitch) + x;
        const auto destination =
            (cellY + height - 1 - y) * atlasWidth + cellX + x;
        pixels[destination] = slot->bitmap.buffer[source];
      }
    }

    auto& glyph = mGlyphs[code];
    glyph.size = {static_cast<float>(width), static_cast<float>(height)};
    glyph.bearing = {static_cast<float>(slot->bitmap_left),
                     static_cast<float>(slot->bitmap_top)};
    glyph.uvOffset = {static_cast<float>(cellX) / atlasWidth,
                      static_cast<float>(cellY) / atlasHeight};
    glyph.uvScale = {static_cast<float>(width) / atlasWidth,
                     static_cast<float>(height) / atlasHeight};
    glyph.advance = static_cast<float>(slot->advance.x) / 64.0f;
  }

  mLineHeight = static_cast<float>(face->size->metrics.height) / 64.0f;
  FT_Done_Face(face);
  FT_Done_FreeType(library);

  if (!valid) {
    return;
  }

  mAtlas = std::make_shared<Texture>(atlasWidth, atlasHeight, 1, pixels.data(),
                                     TextureFilter::Linear, true);
  float vertices[]{0.5f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f,
                   -0.5f, -0.5f, 0.0f, -0.5f, 0.5f, 0.0f};
  float texcoords[]{1.0f, 1.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f};
  std::uint32_t elements[]{0, 3, 1, 1, 3, 2};
  mMesh = std::make_shared<Mesh>(vertices, 4, 3, texcoords, elements, 6);
  mLoaded = mAtlas->IsLoadedFromSource();
}

const Glyph& Font::GetGlyph(unsigned char character) const {
  const unsigned char supported =
      character >= FirstCharacter && character <= LastCharacter
          ? character
          : static_cast<unsigned char>('?');
  return mGlyphs[supported];
}

} // namespace eclipse::graphics
