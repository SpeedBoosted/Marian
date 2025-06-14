// klasy.cpp
#include "klasy.h"
#include <SFML/Graphics.hpp>  // potrzebne m.in. sf::Keyboard
#include <cmath>

// fizyka
static const float GRAVITY = 0.5f;
static const float JUMP_SPEED = -10.f;

// --- Platform ---
Platform::Platform(float x, float y, float width, float height) {
    shape.setPosition(x, y);
    shape.setSize({ width, height });
    shape.setFillColor(sf::Color::Green);
}

// --- MovingPlatform ---
MovingPlatform::MovingPlatform(
    float x, float y, float w, float h,
    const sf::Vector2f& vel, float travelDist)
    : Platform(x, y, w, h), origin(x, y),
    velocity(vel), travel(travelDist)
{
}

void MovingPlatform::update() {
    shape.move(velocity);
    sf::Vector2f offs = shape.getPosition() - origin;
    if (std::abs(offs.x) > travel) velocity.x = -velocity.x;
    if (std::abs(offs.y) > travel) velocity.y = -velocity.y;
}

// --- Bullet ---
Bullet::Bullet(float x, float y, float vx, float vy)
    : velocity(vx, vy), active(true)
{
    shape.setPosition(x, y);
    shape.setSize({ 10.f, 5.f });
    shape.setFillColor(sf::Color::Yellow);
}

void Bullet::update() {
    if (active) shape.move(velocity);
}

// --- Player ---
Player::Player(float x, float y)
    : velocity(0.f, 0.f), onGround(false), hp(100)
{
    shape.setPosition(x, y);
    shape.setSize({ 40.f,60.f });
    shape.setFillColor(sf::Color::Red);
    hpBar.setSize({ (float)hp,10.f });
    hpBar.setFillColor(sf::Color::Green);
    hpBar.setPosition(x, y - 15.f);
}

void Player::update(const std::vector<Platform*>& platforms) {
    velocity.y += GRAVITY;
    shape.move(velocity);
    hpBar.setPosition(shape.getPosition().x,
        shape.getPosition().y - 15.f);
    onGround = false;
    auto b = shape.getGlobalBounds();
    for (auto* p : platforms) {
        auto pb = p->shape.getGlobalBounds();
        if (b.intersects(pb) && velocity.y >= 0.f) {
            shape.setPosition(b.left, pb.top - b.height);
            velocity.y = 0.f;
            onGround = true;
        }
    }
    for (auto& bl : bullets) bl.update();
}

void Player::jump() {
    if (onGround) {
        velocity.y = JUMP_SPEED;
        onGround = false;
    }
}

void Player::move(float dx) {
    shape.move(dx, 0.f);
}

void Player::takeDamage(int amount) {
    hp -= amount; if (hp < 0) hp = 0;
    hpBar.setSize({ (float)hp,10.f });
}

void Player::shoot(const sf::Vector2f& target) {
    sf::Vector2f pos = shape.getPosition();
    pos.x += shape.getSize().x / 2.f;
    pos.y += shape.getSize().y / 2.f;
    float dx = target.x - pos.x, dy = target.y - pos.y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len <= 0.f) return;
    dx /= len; dy /= len;
    bullets.emplace_back(pos.x, pos.y, dx * 8.f, dy * 8.f);
}

// --- Enemy ---
Enemy::Enemy(float x, float y, Type t)
    : speed(2.f), alive(true), hp(100), type(t)
{
    shape.setPosition(x, y);
    shape.setSize({ 40.f,40.f });
    shape.setFillColor(sf::Color(139, 69, 19));
}

void Enemy::update(const std::vector<Platform*>& platforms,
    const Player& player,
    sf::Sound& shootSound)
{
    if (!alive) return;
    bool onPlat = false; const Platform* base = nullptr;
    auto eB = shape.getGlobalBounds();
    for (auto* p : platforms) {
        auto pB = p->shape.getGlobalBounds();
        float eBot = eB.top + eB.height;
        if (std::abs(eBot - pB.top) < 5.f &&
            eB.left + eB.width > pB.left + 2 &&
            eB.left < pB.left + pB.width - 2)
        {
            onPlat = true; base = p;
            shape.setPosition(eB.left, pB.top - eB.height);
            break;
        }
    }

    if (onPlat) {
        auto pB = base->shape.getGlobalBounds();
        float nextX = shape.getPosition().x + speed;
        if (nextX < pB.left || nextX + eB.width > pB.left + pB.width)
            speed = -speed;
        else
            shape.move(speed, 0.f);
    }
    else {
        shape.move(0.f, GRAVITY);
    }

    for (auto& bl : bullets) bl.update();
    if (rand() % 120 == 0) {
        if (type == PISTOL)      shootPistol(player);
        else                    shootShotgun(player);
        shootSound.play();
    }
}

void Enemy::shootPistol(const Player& player) {
    sf::Vector2f e = shape.getPosition();
    e.x += shape.getSize().x / 2.f; e.y += shape.getSize().y / 2.f;
    auto p = player.shape.getPosition();
    float dx = p.x - e.x, dy = p.y - e.y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len <= 0.f) return;
    dx /= len; dy /= len;
    bullets.emplace_back(e.x, e.y, dx * 5.f, dy * 5.f);
}

void Enemy::shootShotgun(const Player& player) {
    sf::Vector2f e = shape.getPosition();
    e.x += shape.getSize().x / 2.f; e.y += shape.getSize().y / 2.f;
    auto p = player.shape.getPosition();
    float base = std::atan2(p.y - e.y, p.x - e.x);
    const int pellets = 5; const float spr = 15.f * 3.14159f / 180.f, spd = 6.f;
    for (int i = 0;i < pellets;++i) {
        float ang = base + (i - pellets / 2) * spr;
        bullets.emplace_back(e.x, e.y, std::cos(ang) * spd, std::sin(ang) * spd);
    }
}

void Enemy::takeDamage(int amount) {
    hp -= amount; if (hp <= 0) { alive = false; hp = 0; }
}

// --- Menu ---
Menu::Menu() : inMenu(true), selectedLevel(0) {
    font.loadFromFile("arial.ttf");
    t1.setFont(font); t1.setString("1: Level 1"); t1.setPosition(300.f, 200.f);
    t2.setFont(font); t2.setString("2: Level 2"); t2.setPosition(300.f, 300.f);
    t3.setFont(font); t3.setString("3: Level 3"); t3.setPosition(300.f, 400.f);
}

void Menu::handleInput() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
        selectedLevel = 1; inMenu = false;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
        selectedLevel = 2; inMenu = false;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
        selectedLevel = 3; inMenu = false;
    }
}

void Menu::draw(sf::RenderWindow& window) {
    window.draw(t1);
    window.draw(t2);
    window.draw(t3);
}
