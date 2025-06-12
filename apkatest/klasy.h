#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Platform {
public:
    sf::RectangleShape shape;
    Platform(float x, float y, float width, float height);
    void draw(sf::RenderWindow& window);
};

class Bullet {
public:
    sf::CircleShape shape;
    sf::Vector2f velocity;
    bool active;
    bool fromEnemy;

    Bullet(float x, float y, float vx, float vy, bool enemy);
    void update();
    void draw(sf::RenderWindow& window);
};

class Player {
public:
    sf::RectangleShape shape;
    float yVelocity;
    int hp;
    bool onGround;

    Player(float x, float y);
    void move(float dx, const std::vector<Platform>& platforms);
    void update(const std::vector<Platform>& platforms);
    bool jump(const std::vector<Platform>& platforms);
    Bullet shoot();
    void draw(sf::RenderWindow& window);
};

class Enemy {
public:
    sf::RectangleShape shape;
    Enemy(float x, float y);
    Bullet shootTowards(const Player& player);
    void draw(sf::RenderWindow& window);
};
