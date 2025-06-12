#pragma once
#include "klasy.h"

struct LevelData {
    std::vector<Platform> platforms;
    std::vector<Enemy> enemies;
    sf::Vector2f playerStart;
    sf::Vector2f goalPos;
};

LevelData createLevel1();
LevelData createLevel2();
LevelData createLevel3();
