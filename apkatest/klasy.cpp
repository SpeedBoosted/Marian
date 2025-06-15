// klasy.cpp
#include "klasy.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <iostream>

using namespace sf;
using namespace std;

// fizyka
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
    : Platform(x, y, w, h),
    origin(x, y),
    velocity(vel),
    travel(t)
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

void Bullet::update() {
    if (active)
        shape.move(velocity);
}

// --- Player ---
Texture Player::runTexture;

Player::Player(float x, float y)
    : velocity(0.f, 0.f),
    onGround(false),
    hp(100)
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
    // animacja biegu
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
    shape.move(0.f, velocity.y);

    // aktualizacja paska HP
    Vector2f pos = shape.getPosition();
    hpBar.setPosition(pos.x - shape.getSize().x / 2.f,
        pos.y - shape.getSize().y / 2.f - 15.f);

    // kolizje z platformami
    onGround = false;
    FloatRect gb = shape.getGlobalBounds();
    for (auto* p : plats) {
        FloatRect pb = p->shape.getGlobalBounds();
        if (gb.intersects(pb) && velocity.y >= 0.f) {
            shape.setPosition(gb.left + gb.width / 2.f,
                pb.top - gb.height / 2.f);
            velocity.y = 0.f;
            onGround = true;
            break;
        }
    }

    // aktualizacja pocisków
    for (auto& b : bullets)
        b.update();

    velocity.x = 0.f;
}

void Player::jump() {
    if (onGround) {
        velocity.y = JUMP_SPEED;
        onGround = false;
    }
}

void Player::takeDamage(int amt) {
    hp -= amt;
    if (hp < 0) hp = 0;
    hpBar.setSize(Vector2f((float)hp, 10.f));
}

void Player::shoot(const Vector2f& tgt) {
    Vector2f c = shape.getPosition();
    float dx = tgt.x - c.x;
    float dy = tgt.y - c.y;
    float len = sqrt(dx * dx + dy * dy);
    if (len > 0.f)
        bullets.emplace_back(c.x, c.y, dx / len * 8.f, dy / len * 8.f);
}

// --- Enemy ---
Enemy::Enemy(float x, float y, Type t)
    : speed(2.f),
    alive(true),
    hp(100),
    type(t)
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

    // poruszanie się po platformie
    bool onPlat = false;
    const Platform* base = nullptr;
    FloatRect eB = shape.getGlobalBounds();
    for (auto* p : plats) {
        FloatRect pB = p->shape.getGlobalBounds();
        float eBot = eB.top + eB.height;
        if (abs(eBot - pB.top) < 5.f &&
            eB.left + eB.width > pB.left + 2.f &&
            eB.left < pB.left + pB.width - 2.f)
        {
            onPlat = true;
            base = p;
            shape.setPosition(eB.left, pB.top - eB.height);
            break;
        }
    }

    if (onPlat) {
        FloatRect pB = base->shape.getGlobalBounds();
        float nx = shape.getPosition().x + speed;
        if (nx < pB.left || nx + eB.width > pB.left + pB.width)
            speed = -speed;
        else
            shape.move(speed, 0.f);
    }
    else {
        shape.move(0.f, GRAVITY);
    }

    // aktualizacja pocisków wroga
    for (auto& b : bullets)
        b.update();

    // strzelanie co losową ilość klatek
    if (rand() % 120 == 0) {
        if (type == PISTOL)
            shootPistol(pl);
        else
            shootShotgun(pl);
        snd.play();
    }
}

void Enemy::shootPistol(const Player& pl) {
    Vector2f e = shape.getPosition() + shape.getSize() / 2.f;
    Vector2f p = pl.shape.getPosition();
    float dx = p.x - e.x;
    float dy = p.y - e.y;
    float len = sqrt(dx * dx + dy * dy);
    if (len > 0.f)
        bullets.emplace_back(e.x, e.y, dx / len * 5.f, dy / len * 5.f);
}

void Enemy::shootShotgun(const Player& pl) {
    Vector2f e = shape.getPosition() + shape.getSize() / 2.f;
    Vector2f p = pl.shape.getPosition();
    float base = atan2(p.y - e.y, p.x - e.x);
    const int pellets = 5;
    const float spread = 15.f * 3.14159f / 180.f;
    const float speed = 6.f;
    for (int i = 0; i < pellets; i++) {
        float ang = base + (i - pellets / 2) * spread;
        bullets.emplace_back(e.x, e.y, cos(ang) * speed, sin(ang) * speed);
    }
}

void Enemy::takeDamage(int amt) {
    hp -= amt;
    if (hp <= 0) {
        alive = false;
        hp = 0;
    }
}

// --- Menu ---
Menu::Menu()
    : inMenu(true),
    selectedLevel(0)
{
    if (!font.loadFromFile("flame.otf"))
        cerr << "flame.otf missing\n";

    t1.setFont(font); t1.setString("1: Level 1"); t1.setPosition(300, 200);
    t2.setFont(font); t2.setString("2: Level 2"); t2.setPosition(300, 300);
    t3.setFont(font); t3.setString("3: Level 3"); t3.setPosition(300, 400);

    if (!bgTexture.loadFromFile("tlo_menu.png"))
        cerr << "tlo_menu.png missing\n";
    bgSprite.setTexture(bgTexture);
}

void Menu::handleInput() {
    if (Keyboard::isKeyPressed(Keyboard::Num1)) {
        selectedLevel = 1; inMenu = false;
    }
    if (Keyboard::isKeyPressed(Keyboard::Num2)) {
        selectedLevel = 2; inMenu = false;
    }
    if (Keyboard::isKeyPressed(Keyboard::Num3)) {
        selectedLevel = 3; inMenu = false;
    }
}

void Menu::draw(sf::RenderWindow& w) {
    // dopasowanie tła do rozmiaru okna
    Vector2u ws = w.getSize();
    float sx = float(ws.x) / bgTexture.getSize().x;
    float sy = float(ws.y) / bgTexture.getSize().y;
    bgSprite.setScale(sx, sy);

    w.draw(bgSprite);
    w.draw(t1);
    w.draw(t2);
    w.draw(t3);
}
