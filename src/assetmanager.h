#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H
#include <string>
#include <map>
#include "SDL_ttf.h"
#include "texturemanager.h"
#include "entitymanager.h"
#include "fontmanager.h" 
class AssetManager {
  private:
    EntityManager *manager;
    std::map<std::string,SDL_Texture*> textures;
    std::map<std::string, TTF_Font*> fonts;

  public:
    AssetManager(EntityManager *manager);
    ~AssetManager();
    void clearData();
    void AddTexture(std::string textureId, const char *filePath);
    SDL_Texture *GetTexture(std::string textureId);
    void AddFont(std::string fontId,const char *filePath,int fontSize);
    TTF_Font *GetFont(std::string fontId);
};


#endif
