#pragma once
  
#include <vector>
#include "klasy.h"
 
struct LevelData {
    std::vector<Platform> platforms;
    std::vector<Enemy> enemies;
};

// Deklaracje funkcji tworzących poziomy
LevelData createLevel1();
LevelData createLevel2();
LevelData createLevel3();
