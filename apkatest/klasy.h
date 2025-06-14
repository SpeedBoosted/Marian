#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>

class Platform {
public:
    sf::RectangleShape shape;
    Platform(float x, float y, float width, float height);
};

class Bullet {
public:
    sf::RectangleShape shape;
    sf::Vector2f       velocity;
    bool               active;

    Bullet(float x, float y, float vx, float vy);
    void update();
};

class Player {
public:
    sf::RectangleShape  shape;
    sf::RectangleShape  hpBar;
    sf::Vector2f        velocity;
    bool                onGround;
    int                 hp;
    std::vector<Bullet> bullets;

    Player(float x = 0, float y = 0);
    void update(const std::vector<Platform>& platforms);
    void jump();
    void move(float dx);
    void takeDamage(int amount);
    void shoot(const sf::Vector2f& target);
};

class Enemy {
public:
    sf::RectangleShape  shape;
    float               speed;
    bool                alive;
    int                 hp;
    std::vector<Bullet> bullets;

    Enemy(float x = 0, float y = 0);
    // Teraz update przyjmuje referencjê do Sound
    void update(const std::vector<Platform>& platforms,
        const Player& player,
        sf::Sound& enemyShootSound);
    void shoot(const Player& player);
    void takeDamage(int amount);
};

class Menu {
public:
    sf::Font  font;
    sf::Text  t1, t2, t3;
    bool      inMenu;
    int       selectedLevel;

    Menu();
    void handleInput();
    void draw(sf::RenderWindow& window);
};
