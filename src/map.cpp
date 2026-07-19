#include "map.h"
#include "entitymanager.h"
#include "tilecomponent.h"
#include <fstream>

extern EntityManager manager;


Map::Map(std::string textureId,int scale, int tileSize){
  this -> textureId = textureId;
  this -> scale = scale;
  this -> tileSize = tileSize;
}

void Map::LoadMap(std::string filePath, int mapSizeX, int mapSizeY) {
  std::fstream mapFile;
  mapFile.open(filePath);
  for (int y = 0; y < mapSizeY; y++) {
    for (int x = 0; x < mapSizeX; x++) {
      char ch;
      mapFile.get(ch);
      int sourceRectangleY = atoi(&ch) * tileSize;
      mapFile.get(ch);
      int sourceRectangleX = atoi(&ch) * tileSize;
      AddTile(sourceRectangleX, sourceRectangleY, x * (scale * tileSize),
              y * (scale * tileSize));
      mapFile.ignore();
    }
  }
  mapFile.close();
}

void Map::AddTile(int sourceRectangleX, int sourceRectangleY, int x , int y){
  Entity &newTile(manager.AddEntity("Tile",TILEMAP_LAYER));
  newTile.AddComponent<TileComponent>(sourceRectangleX,sourceRectangleY,x,y,tileSize,scale,textureId);

  
}
