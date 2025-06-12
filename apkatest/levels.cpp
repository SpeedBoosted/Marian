#include "levels.h"

LevelData createLevel1() {
    LevelData level;
    level.playerStart = { 100, 600 };
    level.goalPos = { 1100, 600 };
    level.platforms.push_back(Platform(0, 680, 1280, 40));
    level.platforms.push_back(Platform(300, 600, 100, 20));
    level.enemies.push_back(Enemy(800, 640));
    return level;
}

LevelData createLevel2() {
    LevelData level;
    level.playerStart = { 100, 600 };
    level.goalPos = { 1150, 500 };
    level.platforms.push_back(Platform(0, 680, 1280, 40));
    level.platforms.push_back(Platform(400, 600, 100, 20));
    level.platforms.push_back(Platform(600, 500, 100, 20));
    level.enemies.push_back(Enemy(900, 640));
    return level;
}

LevelData createLevel3() {
    LevelData level;
    level.playerStart = { 50, 600 };
    level.goalPos = { 1150, 400 };
    level.platforms.push_back(Platform(0, 680, 1280, 40));
    level.platforms.push_back(Platform(300, 600, 100, 20));
    level.platforms.push_back(Platform(500, 500, 100, 20));
    level.platforms.push_back(Platform(700, 400, 100, 20));
    level.enemies.push_back(Enemy(1000, 640));
    return level;
}
