// klasy.h
#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>

class Platform {
public:
    sf::RectangleShape shape;
    Platform(float x, float y, float width, float height);
    virtual void update() {}
};

class MovingPlatform : public Platform {
public:
    sf::Vector2f origin, velocity;
    float        travel;
    MovingPlatform(float x, float y, float w, float h,
        const sf::Vector2f& vel, float travelDist);
    void update() override;
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
    sf::RectangleShape  shape, hpBar;
    sf::Vector2f        velocity;
    bool                onGround;
    int                 hp;
    std::vector<Bullet> bullets;
    Player(float x = 0.f, float y = 0.f);
    void update(const std::vector<Platform*>& platforms);
    void jump();
    void move(float dx);
    void takeDamage(int amount);
    void shoot(const sf::Vector2f& target);
};

class Enemy {
public:
    enum Type { PISTOL, SHOTGUN };
    sf::RectangleShape  shape;
    float               speed;
    bool                alive;
    int                 hp;
    Type                type;
    std::vector<Bullet> bullets;
    Enemy(float x = 0.f, float y = 0.f, Type t = PISTOL);
    void update(const std::vector<Platform*>& platforms,
        const Player& player,
        sf::Sound& shootSound);
    void takeDamage(int amount);
private:
    void shootPistol(const Player& player);
    void shootShotgun(const Player& player);
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
