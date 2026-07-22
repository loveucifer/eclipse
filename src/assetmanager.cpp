#include "assetmanager.h"
#include "SDL_ttf.h"

AssetManager::AssetManager (EntityManager *manager):manager(manager){
  
}

void AssetManager::clearData(){
  textures.clear();  // clears the map
  fonts.clear();
}

void AssetManager::AddFont(std::string fontId, const char *filePath,
                           int fontSize) {

  fonts.emplace(fontId, FontManager::LoadFont(filePath, fontSize));
}

void AssetManager::AddTexture(std::string textureId, const char *filePath) {
  textures.emplace(textureId, TextureManager::LoadTexture(filePath));
}

SDL_Texture *AssetManager::GetTexture(std::string textureId) {
  return textures[textureId];
}

TTF_Font *AssetManager::GetFont(std::string fontId) { return fonts[fontId]; }
