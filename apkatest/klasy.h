#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <map>
#include <string>

class Player; // forward

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
    sf::Vector2f         velocity;
    bool                 active;
    int damage = 10;
    Bullet(float x, float y, float vx, float vy);
    void update();
};

enum class AlcoholType { Piwo, Wodka, Kubus, Wino };

class Alcohol {
public:
    Alcohol(AlcoholType type, float x, float y);
    AlcoholType type;
    sf::RectangleShape shape;
    bool picked = false;

    // Statyczne tekstury dla każdego typu alkoholu
    static sf::Texture beerTexture;
    static sf::Texture wodkaTexture;
    static sf::Texture kubusTexture;
    static sf::Texture wineTexture;
};

class Player {
public:
    sf::RectangleShape shape, hpBar;
    sf::Vector2f         velocity;
    bool                 onGround;
    int                  hp;
    std::vector<Bullet> bullets;

    static sf::Texture runTexture;
    int    frameCount = 10;
    float  timePerFrame = 0.08f;
    int    currentFrame = 0;
    sf::Clock animClock;
    sf::Clock damageClock; // Dodany clock do obsługi hazardów

    Player(float x = 0, float y = 0);
    void update(const std::vector<Platform*>& plats);
    void jump();
    void move(float dx);
    void takeDamage(int amt);
    void shoot(const sf::Vector2f& tgt);
    std::map<AlcoholType, int> alcoholInventory;
    AlcoholType selectedAlcohol = AlcoholType::Piwo;
    float jumpBoost = 0;
    float speedBoost = 1;
    float damageBoost = 1;
    float cooldownPenalty = 1;

    void pickUpAlcohol(std::vector<Alcohol>& drinks);
    void useAlcohol();
    void resetAlcoholEffects();

    sf::FloatRect getCollisionBounds() const;
};

class Enemy {
public:
    enum Type { PISTOL, SHOTGUN, BOSS };
    enum State { Patrol, Chase, Attack, Retreat };

    sf::RectangleShape    shape;
    int hits = 0;
    float                 speed;
    bool                  alive;
    int                   hp;
    Type                  type;
    State                 state;
    float                 detectionRange;
    int                   retreatThreshold;
    int                   chaseThreshold;
    float                 shootCooldown;
    sf::Clock             shootClock;
    std::vector<Bullet>   bullets;
    float bossTime = 0.f; // tylko dla bossa te 3 som
    sf::Vector2f bossCenter;
    float bossRadius = 120.f;

    // Nowe zmienne dla animacji wroga PISTOL
    static sf::Texture pistolEnemyTexture; 
    int pistolFrameCount = 10; 
    float pistolTimePerFrame = 0.1f; 
    int currentPistolFrame = 0;
    sf::Clock pistolAnimClock;

 
    static sf::Texture shotgunEnemyTexture; 
    int shotgunFrameCount = 10; 
    float shotgunTimePerFrame = 0.1f; // Czas na klatkę (dostosuj)
    int currentShotgunFrame = 0;
    sf::Clock shotgunAnimClock;

    Enemy(float x = 0, float y = 0, Type t = PISTOL);
    void update(const std::vector<Platform*>& plats,
        const Player& pl,
        sf::Sound& snd);
    void takeDamage(int amt);

private:
    void shootPistol(const Player& pl);
    void shootShotgun(const Player& pl);
    void shootBossAttack(const Player& pl);
    bool canSeePlayer(const Player& pl,
        const std::vector<Platform*>& plats);
};

class Hazard {
public:
    sf::RectangleShape shape;
    static sf::Texture  texture; // Dodano statyczną teksturę
    Hazard(float x, float y, float w, float h);
    void update(Player& p);

private:
    sf::Clock damageClock;
};

class Menu {
public:
    sf::Font    font;
    sf::Text    t1, t2, t3, t4, t5;
    bool        inMenu;
    int         selectedLevel;
    sf::Texture bgTexture;
    sf::Sprite  bgSprite;

    Menu();
    void handleInput();
    void draw(sf::RenderWindow& w);
};