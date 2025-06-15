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
    shape.setSize({ w,h });
    shape.setPosition(x, y);
    shape.setTextureRect({ 0,0,int(w),int(h) });
}

// --- MovingPlatform ---
MovingPlatform::MovingPlatform(float x, float y, float w, float h,
    const Vector2f& vel, float t)
    : Platform(x, y, w, h), origin(x, y), velocity(vel), travel(t)
{
}
void MovingPlatform::update() {
    shape.move(velocity);
    auto offs = shape.getPosition() - origin;
    if (abs(offs.x) > travel) velocity.x = -velocity.x;
    if (abs(offs.y) > travel) velocity.y = -velocity.y;
}

// --- Bullet ---
Bullet::Bullet(float x, float y, float vx, float vy)
    : velocity(vx, vy), active(true)
{
    shape.setPosition(x, y);
    shape.setSize({ 10,5 });
    shape.setFillColor(Color::Yellow);
}
void Bullet::update() {
    if (active) shape.move(velocity);
}

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
    shape.setSize({ float(fw),float(fh) });
    shape.setTextureRect({ 0,0,fw,fh });
    shape.setOrigin(fw / 2.f, fh / 2.f);
    shape.setPosition(x + fw / 2.f, y + fh / 2.f);

    hpBar.setSize({ float(hp),10 });
    hpBar.setFillColor(Color::Green);
    hpBar.setPosition(x, y - 15.f);
}
void Player::move(float dx) {
    velocity.x = dx; shape.move(dx, 0);
}
void Player::update(const vector<Platform*>& plats) {
    if (velocity.x != 0) {
        if (animClock.getElapsedTime().asSeconds() > timePerFrame) {
            animClock.restart();
            currentFrame = (currentFrame + 1) % frameCount;
            int fw = runTexture.getSize().x / frameCount;
            int fh = runTexture.getSize().y;
            shape.setTextureRect({ currentFrame * fw,0,fw,fh });
        }
        shape.setScale((velocity.x < 0 ? -1.f : 1.f), 1.f);
    }
    else {
        shape.setScale(1, 1);
        int fw = runTexture.getSize().x / frameCount;
        int fh = runTexture.getSize().y;
        shape.setTextureRect({ 0,0,fw,fh });
    }

    velocity.y += GRAVITY;
    shape.move(0, velocity.y);

    auto pos = shape.getPosition();
    hpBar.setPosition(pos.x - shape.getSize().x / 2.f,
        pos.y - shape.getSize().y / 2.f - 15.f);

    onGround = false;
    auto gb = shape.getGlobalBounds();
    for (auto* p : plats) {
        auto pb = p->shape.getGlobalBounds();
        if (gb.intersects(pb) && velocity.y >= 0) {
            shape.setPosition(gb.left + gb.width / 2.f,
                pb.top - gb.height / 2.f);
            velocity.y = 0; onGround = true; break;
        }
    }
    for (auto& b : bullets) b.update();
    velocity.x = 0;
}
void Player::jump() {
    if (onGround) { velocity.y = JUMP_SPEED; onGround = false; }
}
void Player::takeDamage(int amt) {
    hp = max(0, hp - amt);
    hpBar.setSize({ float(hp),10 });
}
void Player::shoot(const Vector2f& tgt) {
    auto c = shape.getPosition();
    float dx = tgt.x - c.x, dy = tgt.y - c.y;
    float len = sqrt(dx * dx + dy * dy);
    if (len > 0) bullets.emplace_back(c.x, c.y, dx / len * 8.f, dy / len * 8.f);
}
sf::FloatRect Player::getCollisionBounds() const {
    auto b = shape.getGlobalBounds();
    float sx = b.width * 0.7f, sy = b.height * 0.6f;
    b.left += sx / 2; b.top += sy / 2; b.width -= sx; b.height -= sy;
    return b;
}

// --- Enemy ---
Enemy::Enemy(float x, float y, Type t)
    : speed(2.f), alive(true), hp(100), type(t),
    state(Patrol), detectionRange(500.f),
    retreatThreshold(30), chaseThreshold(60)
{
    shape.setPosition(x, y);
    shape.setSize({ 40,40 });
    shape.setFillColor(Color(139, 69, 19));
    shootCooldown = (type == PISTOL ? 0.8f : 1.5f);
    shootClock.restart();
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
        if (abs(eBot - pB.top) < 5.f &&
            eB.left + eB.width > pB.left + 2 &&
            eB.left < pB.left + pB.width - 2)
        {
            onPlat = true; base = p;
            shape.setPosition(eB.left, pB.top - eB.height);
            break;
        }
    }
    if (!onPlat) shape.move(0, GRAVITY);

    auto eC = shape.getPosition() + shape.getSize() / 2.f;
    auto pC = pl.shape.getPosition();
    float dx = pC.x - eC.x, dy = pC.y - eC.y;
    float dist = sqrt(dx * dx + dy * dy);
    bool see = canSeePlayer(pl, plats) && dist < detectionRange;

    switch (state) {
    case Patrol:
        if (onPlat) {
            auto pB = base->shape.getGlobalBounds();
            float nx = shape.getPosition().x + speed;
            if (nx<pB.left || nx + eB.width>pB.left + pB.width) speed = -speed;
            else shape.move(speed, 0);
        }
        if (see) state = Chase;
        break;
    case Chase:
        if (!see) { state = Patrol; break; }
        shape.move((dx > 0 ? 1.f : -1.f) * speed, 0);
        state = Attack;
        break;
    case Attack:
        if (!see) { state = Patrol; break; }
        if (hp < retreatThreshold) { state = Retreat; break; }
        if (shootClock.getElapsedTime().asSeconds() >= shootCooldown) {
            if (type == PISTOL) shootPistol(pl);
            else             shootShotgun(pl);
            snd.play();
            shootClock.restart();
        }
        break;
    case Retreat:
        if (hp > chaseThreshold) { state = Patrol; break; }
        shape.move((dx > 0 ? -1.f : 1.f) * speed, 0);
        break;
    }
    for (auto& b : bullets) b.update();
}
void Enemy::shootPistol(const Player& pl) {
    auto e = shape.getPosition() + shape.getSize() / 2.f;
    auto p = pl.shape.getPosition();
    float dx = p.x - e.x, dy = p.y - e.y;
    float len = sqrt(dx * dx + dy * dy);
    if (len > 0) bullets.emplace_back(e.x, e.y, dx / len * 5.f, dy / len * 5.f);
}
void Enemy::shootShotgun(const Player& pl) {
    auto e = shape.getPosition() + shape.getSize() / 2.f;
    auto p = pl.shape.getPosition();
    float baseAng = atan2(p.y - e.y, p.x - e.x);
    const int pellets = 5;
    const float spread = 15 * 3.14159f / 180.f, spd = 6.f;
    for (int i = 0;i < pellets;i++) {
        float ang = baseAng + (i - pellets / 2) * spread;
        bullets.emplace_back(e.x, e.y, cos(ang) * spd, sin(ang) * spd);
    }
}
void Enemy::takeDamage(int amt) {
    hp = max(0, hp - amt);
    if (hp == 0) alive = false;
}
bool Enemy::canSeePlayer(const Player& pl,
    const vector<Platform*>& plats)
{
    auto E = shape.getPosition() + shape.getSize() / 2.f;
    auto P = pl.shape.getPosition();
    auto dir = P - E; float len = sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len == 0) return true;
    dir /= len;
    for (float d = 0;d < len;d += 5.f) {
        auto sample = E + dir * d;
        for (auto* p : plats) {
            if (p->shape.getGlobalBounds().contains(sample))
                return false;
        }
    }
    return true;
}

// --- Hazard ---
Hazard::Hazard(float x, float y, float w, float h) {
    shape.setSize({ w,h });
    shape.setFillColor(Color::Red);
    shape.setPosition(x, y);
    damageClock.restart();
}
void Hazard::update(Player& p) {
    if (shape.getGlobalBounds().intersects(p.shape.getGlobalBounds())) {
        if (damageClock.getElapsedTime().asSeconds() >= 1.f) {
            p.takeDamage(10);
            damageClock.restart();
        }
    }
    else {
        damageClock.restart();
    }
}

// --- Menu ---
Menu::Menu() :inMenu(true), selectedLevel(0) {
    if (!font.loadFromFile("flame.otf")) cerr << "flame.otf missing\n";
    t1.setFont(font); t1.setString("1: Level 1"); t1.setPosition(300, 200);
    t2.setFont(font); t2.setString("2: Level 2"); t2.setPosition(300, 300);
    t3.setFont(font); t3.setString("3: Level 3"); t3.setPosition(300, 400);
    if (!bgTexture.loadFromFile("tlo_menu.png"))
        cerr << "tlo_menu.png missing\n";
    bgSprite.setTexture(bgTexture);
}
void Menu::handleInput() {
    if (Keyboard::isKeyPressed(Keyboard::Num1))
    {
        selectedLevel = 1; inMenu = false;
    }
    if (Keyboard::isKeyPressed(Keyboard::Num2))
    {
        selectedLevel = 2; inMenu = false;
    }
    if (Keyboard::isKeyPressed(Keyboard::Num3))
    {
        selectedLevel = 3; inMenu = false;
    }
}
void Menu::draw(RenderWindow& w) {
    auto ws = w.getSize();
    float sx = float(ws.x) / bgTexture.getSize().x;
    float sy = float(ws.y) / bgTexture.getSize().y;
    bgSprite.setScale(sx, sy);
    w.draw(bgSprite);
    w.draw(t1); w.draw(t2); w.draw(t3);
}
