// klasy.cpp
#include "klasy.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <iostream>

using namespace sf;
using namespace std;

static const float GRAVITY = 0.5f;
static const float JUMP_SPEED = -10.f;

// --- Platform ---
Texture Platform::texture;
Platform::Platform(float x, float y, float w, float h) {
    if (texture.getSize().x == 0) {
        if (!texture.loadFromFile("platform.png"))
            cerr << "platform.png missing\n";
        texture.setRepeated(true);
    }
    shape.setTexture(&texture);
    shape.setSize(Vector2f(w, h));
    shape.setPosition(x, y);
    shape.setTextureRect(IntRect(0, 0, int(w), int(h)));
}

// --- MovingPlatform ---
MovingPlatform::MovingPlatform(float x, float y, float w, float h,
    const Vector2f& vel, float t)
    : Platform(x, y, w, h), origin(x, y), velocity(vel), travel(t)
{
}
void MovingPlatform::update() {
    shape.move(velocity);
    Vector2f offs = shape.getPosition() - origin;
    if (abs(offs.x) > travel) velocity.x = -velocity.x;
    if (abs(offs.y) > travel) velocity.y = -velocity.y;
}

// --- Bullet ---
Bullet::Bullet(float x, float y, float vx, float vy)
    : velocity(vx, vy), active(true)
{
    shape.setPosition(x, y);
    shape.setSize(Vector2f(10.f, 5.f));
    shape.setFillColor(Color::Yellow);
}
void Bullet::update() { if (active) shape.move(velocity); }

// --- Player ---
Texture Player::runTexture;
Player::Player(float x, float y)
    : velocity(0, 0), onGround(false), hp(100)
{
    if (runTexture.getSize().x == 0) {
        if (!runTexture.loadFromFile("player_run.png"))
            cerr << "player_run.png missing\n";
    }
    shape.setTexture(&runTexture);
    int fw = runTexture.getSize().x / frameCount;
    int fh = runTexture.getSize().y;
    shape.setSize(Vector2f((float)fw, (float)fh));
    shape.setTextureRect(IntRect(0, 0, fw, fh));
    shape.setOrigin(fw / 2.f, fh / 2.f);
    shape.setPosition(x + fw / 2.f, y + fh / 2.f);

    hpBar.setSize(Vector2f((float)hp, 10.f));
    hpBar.setFillColor(Color::Green);
    hpBar.setPosition(x, y - 15.f);
}

void Player::move(float dx) {
    velocity.x = dx;
    shape.move(dx, 0.f);
}

void Player::update(const vector<Platform*>& plats) {
    // animacja
    if (velocity.x != 0.f) {
        if (animClock.getElapsedTime().asSeconds() > timePerFrame) {
            animClock.restart();
            currentFrame = (currentFrame + 1) % frameCount;
            int fw = runTexture.getSize().x / frameCount;
            int fh = runTexture.getSize().y;
            shape.setTextureRect(IntRect(currentFrame * fw, 0, fw, fh));
        }
        shape.setScale((velocity.x < 0 ? -1.f : 1.f), 1.f);
    }
    else {
        shape.setScale(1.f, 1.f);
        int fw = runTexture.getSize().x / frameCount;
        int fh = runTexture.getSize().y;
        shape.setTextureRect(IntRect(0, 0, fw, fh));
    }

    // grawitacja
    velocity.y += GRAVITY;
    shape.move(0, velocity.y);

    // pasek HP
    Vector2f p = shape.getPosition();
    hpBar.setPosition(p.x - shape.getSize().x / 2.f,
        p.y - shape.getSize().y / 2.f - 15.f);

    onGround = false;
    auto gb = shape.getGlobalBounds();
    for (auto* p : plats) {
        auto pb = p->shape.getGlobalBounds();
        if (gb.intersects(pb) && velocity.y >= 0) {
            shape.setPosition(gb.left + gb.width / 2.f,
                pb.top - gb.height / 2.f);
            velocity.y = 0; onGround = true;
            break;
        }
    }

    for (auto& b : bullets) b.update();
    velocity.x = 0.f;
}

void Player::jump() {
    if (onGround) { velocity.y = JUMP_SPEED; onGround = false; }
}

void Player::takeDamage(int amt) {
    hp -= amt; if (hp < 0) hp = 0;
    hpBar.setSize(Vector2f((float)hp, 10.f));
}

void Player::shoot(const Vector2f& tgt) {
    Vector2f c = shape.getPosition();
    float dx = tgt.x - c.x, dy = tgt.y - c.y;
    float len = sqrt(dx * dx + dy * dy);
    if (len > 0) bullets.emplace_back(c.x, c.y, dx / len * 8, dy / len * 8);
}

// --- Enemy ---
Enemy::Enemy(float x, float y, Type t)
    : speed(2.f), alive(true), hp(100), type(t)
{
    shape.setPosition(x, y);
    shape.setSize(Vector2f(40.f, 40.f));
    shape.setFillColor(Color(139, 69, 19));
}

void Enemy::update(const vector<Platform*>& plats,
    const Player& pl,
    Sound& snd)
{
    if (!alive) return;
    bool onPlat = false; const Platform* base = nullptr;
    auto eB = shape.getGlobalBounds();
    for (auto* p : plats) {
        auto pB = p->shape.getGlobalBounds();
        float eBot = eB.top + eB.height;
        if (abs(eBot - pB.top) < 5 &&
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
        float nx = shape.getPosition().x + speed;
        if (nx<pB.left || nx + eB.width>pB.left + pB.width) speed = -speed;
        else shape.move(speed, 0);
    }
    else shape.move(0, GRAVITY);

    for (auto& b : bullets) b.update();
    if (rand() % 120 == 0) {
        if (type == PISTOL) shootPistol(pl);
        else               shootShotgun(pl);
        snd.play();
    }
}

void Enemy::shootPistol(const Player& pl) {
    Vector2f e = shape.getPosition() + shape.getSize() / 2.f;
    auto p = pl.shape.getPosition();
    float dx = p.x - e.x, dy = p.y - e.y;
    float len = sqrt(dx * dx + dy * dy);
    if (len > 0) bullets.emplace_back(e.x, e.y, dx / len * 5, dy / len * 5);
}

void Enemy::shootShotgun(const Player& pl) {
    Vector2f e = shape.getPosition() + shape.getSize() / 2.f;
    auto p = pl.shape.getPosition();
    float base = atan2(p.y - e.y, p.x - e.x);
    const int pel = 5; const float spr = 15 * 3.14159f / 180, spd = 6;
    for (int i = 0; i < pel; i++) {
        float ang = base + (i - pel / 2) * spr;
        bullets.emplace_back(e.x, e.y, cos(ang) * spd, sin(ang) * spd);
    }
}

void Enemy::takeDamage(int amt) {
    hp -= amt;
    if (hp <= 0) { alive = false; hp = 0; }
}

// --- Menu ---
Menu::Menu() : inMenu(true), selectedLevel(0) {
    font.loadFromFile("arial.ttf");
    t1.setFont(font); t1.setString("1: Level 1"); t1.setPosition(300, 200);
    t2.setFont(font); t2.setString("2: Level 2"); t2.setPosition(300, 300);
    t3.setFont(font); t3.setString("3: Level 3"); t3.setPosition(300, 400);

    if (!bgTexture.loadFromFile("tlo_menu.png"))
        std::cerr << "Nie znaleziono tlo_menu.png\n";
    bgSprite.setTexture(bgTexture);
}

void Menu::handleInput() {
    if (Keyboard::isKeyPressed(Keyboard::Num1)) { selectedLevel = 1; inMenu = false; }
    if (Keyboard::isKeyPressed(Keyboard::Num2)) { selectedLevel = 2; inMenu = false; }
    if (Keyboard::isKeyPressed(Keyboard::Num3)) { selectedLevel = 3; inMenu = false; }
}

void Menu::draw(sf::RenderWindow& w) {
    Vector2u ws = w.getSize();
    float sx = float(ws.x) / bgTexture.getSize().x;
    float sy = float(ws.y) / bgTexture.getSize().y;
    bgSprite.setScale(sx, sy);

    w.draw(bgSprite);
    w.draw(t1);
    w.draw(t2);
    w.draw(t3);
}
