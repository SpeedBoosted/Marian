// klasy.h
#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>

class Player;  // forward

class Platform {
public:
    sf::RectangleShape shape;
    static sf::Texture  texture;
    Platform(float x, float y, float w, float h);
    virtual void update() {}
};

class MovingPlatform : public Platform {
public:
    sf::Vector2f origin, velocity;
    float travel;
    MovingPlatform(float x, float y, float w, float h,
        const sf::Vector2f& vel, float t);
    void update() override;
};

class Bullet {
public:
    sf::RectangleShape shape;
    sf::Vector2f      velocity;
    bool              active;
    Bullet(float x, float y, float vx, float vy);
    void update();
};

class Player {
public:
    sf::RectangleShape shape, hpBar;
    sf::Vector2f      velocity;
    bool               onGround;
    int                hp;
    std::vector<Bullet> bullets;

    static sf::Texture runTexture;
    int    frameCount = 10;
    float  timePerFrame = 0.08f;
    int    currentFrame = 0;
    sf::Clock animClock;

    Player(float x = 0, float y = 0);
    void update(const std::vector<Platform*>& plats);
    void jump();
    void move(float dx);
    void takeDamage(int amt);
    void shoot(const sf::Vector2f& tgt);

    sf::FloatRect getCollisionBounds() const;
};

class Enemy {
public:
    enum Type { PISTOL, SHOTGUN };
    enum State { Patrol, Chase, Attack, Retreat };

    sf::RectangleShape   shape;
    float                speed;
    bool                 alive;
    int                  hp;
    Type                 type;
    State                state;
    float                detectionRange;
    int                  retreatThreshold;
    int                  chaseThreshold;
    float                shootCooldown;
    sf::Clock            shootClock;
    std::vector<Bullet>  bullets;

    Enemy(float x = 0, float y = 0, Type t = PISTOL);
    void update(const std::vector<Platform*>& plats,
        const Player& pl,
        sf::Sound& snd);
    void takeDamage(int amt);

private:
    void shootPistol(const Player& pl);
    void shootShotgun(const Player& pl);
    bool canSeePlayer(const Player& pl,
        const std::vector<Platform*>& plats);
};

class Hazard {
public:
    sf::RectangleShape shape;
    Hazard(float x, float y, float w, float h);
    void update(Player& p);

private:
    sf::Clock damageClock;
};

class Menu {
public:
    sf::Font   font;
    sf::Text   t1, t2, t3;
    bool       inMenu;
    int        selectedLevel;
    sf::Texture bgTexture;
    sf::Sprite  bgSprite;

    Menu();
    void handleInput();
    void draw(sf::RenderWindow& w);
};
