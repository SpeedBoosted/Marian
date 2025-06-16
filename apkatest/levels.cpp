#include "levels.h"
 
  
LevelData createLevel1() {
    LevelData level;
    level.platforms.emplace_back(sf::Vector2f(0, 550), sf::Vector2f(900, 50));
    level.platforms.emplace_back(sf::Vector2f(200, 450), sf::Vector2f(150, 20));
    level.platforms.emplace_back(sf::Vector2f(400, 350), sf::Vector2f(150, 20));

    level.enemies.emplace_back(sf::Vector2f(600, 490));
    return level;
}

LevelData createLevel2() {
    LevelData level;
    level.platforms.emplace_back(sf::Vector2f(0, 550), sf::Vector2f(900, 50));
    level.platforms.emplace_back(sf::Vector2f(100, 450), sf::Vector2f(200, 20));
    level.platforms.emplace_back(sf::Vector2f(400, 300), sf::Vector2f(100, 20));

    level.enemies.emplace_back(sf::Vector2f(700, 490));
    level.enemies.emplace_back(sf::Vector2f(500, 490));
    return level;
}

LevelData createLevel3() {
    LevelData level;
    level.platforms.emplace_back(sf::Vector2f(0, 550), sf::Vector2f(900, 50));
    level.platforms.emplace_back(sf::Vector2f(300, 450), sf::Vector2f(200, 20));
    level.platforms.emplace_back(sf::Vector2f(600, 350), sf::Vector2f(150, 20));

    level.enemies.emplace_back(sf::Vector2f(750, 490));
    return level;
}
LevelData createLevel4() {
    LevelData level;
    level.platforms.emplace_back(sf::Vector2f(0, 550), sf::Vector2f(2000, 50));
    level.platforms.emplace_back(sf::Vector2f(400, 400), sf::Vector2f(200, 20));
    level.platforms.emplace_back(sf::Vector2f(800, 300), sf::Vector2f(200, 20));
    level.platforms.emplace_back(sf::Vector2f(1400, 400), sf::Vector2f(200, 20));
    // Boss w œrodku poziomu (przyk³ad: Enemy(x, y, typ))
    level.enemies.emplace_back(sf::Vector2f(400, 250), Enemy::BOSS);
    return level;
}

LevelData createLevel5() {
    LevelData level;
    level.platforms.emplace_back(sf::Vector2f(0, 550), sf::Vector2f(2500, 50));
    level.platforms.emplace_back(sf::Vector2f(500, 450), sf::Vector2f(200, 20));
    level.platforms.emplace_back(sf::Vector2f(1000, 350), sf::Vector2f(200, 20));
    level.platforms.emplace_back(sf::Vector2f(1600, 250), sf::Vector2f(200, 20));
    return level;
}