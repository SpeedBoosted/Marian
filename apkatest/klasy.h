#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Platform {
public:
    sf::RectangleShape shape;
    Platform(float x, float y, float width, float height);
};

class Player {
public:
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    bool onGround;

    Player(float x, float y);
    void update(const std::vector<Platform>& platforms);
    void jump();
    void move(float dx);
};

class Enemy {
public:
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    float speed;
    bool alive;

    Enemy(float x, float y);
    void update(const std::vector<Platform>& platforms);
};