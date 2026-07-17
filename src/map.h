#ifndef MAP_H
#define MAP_H
#include <iostream>

class Map {
private:
  std::string textureId; // points to the png
  int scale;             // scale of the map
  int tileSize;

public:
  Map(std::string textureId, int scale, int tileSize);
  ~Map();
  void LoadMap(std::string filePath, int mapSizeX, int mapSizeY);
  void AddTile(int sourceRectangleX, int sourceRectangleY, int x, int y);
};

#endif
