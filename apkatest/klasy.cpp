#include "klasy.h"
#include <cmath>

static const float GRAVITY = 0.5f;
static const float JUMP_SPEED = -10.0f;

// --- Platform ---
Platform::Platform(float x, float y, float width, float height) {
    shape.setPosition(x, y);
    shape.setSize({ width,height });
    shape.setFillColor(sf::Color::Green);
}

// --- Bullet ---
Bullet::Bullet(float x, float y, float vx, float vy) {
    shape.setPosition(x, y);
    shape.setSize({ 10,5 });
    shape.setFillColor(sf::Color::Yellow);
    velocity = { vx,vy };
    active = true;
}

void Bullet::update() {
    if (active)
        shape.move(velocity);
}

// --- Player ---
Player::Player(float x, float y) {
    shape.setPosition(x, y);
    shape.setSize({ 40,60 });
    shape.setFillColor(sf::Color::Red);
    velocity = { 0,0 };
    onGround = false;
    hp = 100;

    hpBar.setSize({ (float)hp,10.f });
    hpBar.setFillColor(sf::Color::Green);
    hpBar.setPosition(x, y - 15.f);
}

void Player::update(const std::vector<Platform>& platforms) {
    // Grawitacja
    velocity.y += GRAVITY;
    shape.move(velocity);

    // Pasek HP
    hpBar.setPosition(shape.getPosition().x,
        shape.getPosition().y - 15.f);

    // Lądowanie na platformach
    onGround = false;
    auto b = shape.getGlobalBounds();
    for (auto& p : platforms) {
        auto pb = p.shape.getGlobalBounds();
        if (b.intersects(pb) && velocity.y >= 0.f) {
            shape.setPosition(b.left, pb.top - b.height);
            velocity.y = 0.f;
            onGround = true;
        }
    }

    // Update pocisków gracza
    for (auto& bl : bullets)
        bl.update();
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
    hp -= amount;
    if (hp < 0) hp = 0;
    hpBar.setSize({ (float)hp,10.f });
}

void Player::shoot(const sf::Vector2f& target) {
    auto pos = shape.getPosition();
    pos.x += shape.getSize().x / 2;
    pos.y += shape.getSize().y / 2;

    float dx = target.x - pos.x;
    float dy = target.y - pos.y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len <= 0.f) return;
    dx /= len; dy /= len;

    bullets.emplace_back(pos.x, pos.y, dx * 8.f, dy * 8.f);
}

// --- Enemy ---
Enemy::Enemy(float x, float y) {
    shape.setPosition(x, y);
    shape.setSize({ 40,40 });
    shape.setFillColor({ 139,69,19 });
    speed = 2.f;
    alive = true;
    hp = 100;
}

void Enemy::update(const std::vector<Platform>& platforms,
    const Player& player,
    sf::Sound& enemyShootSound)
{
    if (!alive) return;

    // Wykrycie platformy
    bool onPlat = false;
    const Platform* pl = nullptr;
    auto eB = shape.getGlobalBounds();
    for (auto& p : platforms) {
        auto pB = p.shape.getGlobalBounds();
        float eBot = eB.top + eB.height;
        if (std::abs(eBot - pB.top) < 5.f &&
            eB.left + eB.width > pB.left + 2 &&
            eB.left < pB.left + pB.width - 2)
        {
            onPlat = true;
            pl = &p;
            shape.setPosition(eB.left, pB.top - eB.height);
            break;
        }
    }

    if (onPlat) {
        // Ruch w bok i odbijanie od krawędzi
        shape.move(speed, 0.f);
        auto pB = pl->shape.getGlobalBounds();
        eB = shape.getGlobalBounds();
        if (eB.left <= pB.left || eB.left + eB.width >= pB.left + pB.width)
            speed = -speed;
    }
    else {
        // Spadanie
        shape.move(0.f, GRAVITY);
    }

    // Update pocisków przeciwnika
    for (auto& bl : bullets)
        bl.update();

    // Strzał co ~120 klatek
    if (rand() % 120 == 0) {
        shoot(player);
        enemyShootSound.play();
    }
}

void Enemy::shoot(const Player& player) {
    auto ePos = shape.getPosition();
    ePos.x += shape.getSize().x / 2;
    ePos.y += shape.getSize().y / 2;

    auto pPos = player.shape.getPosition();
    float dx = pPos.x - ePos.x;
    float dy = pPos.y - ePos.y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len <= 0.f) return;
    dx /= len; dy /= len;

    bullets.emplace_back(ePos.x, ePos.y, dx * 5.f, dy * 5.f);
}

void Enemy::takeDamage(int amount) {
    hp -= amount;
    if (hp <= 0) {
        alive = false;
        hp = 0;
    }
}

// --- Menu ---
Menu::Menu() {
    font.loadFromFile("arial.ttf");
    t1.setFont(font); t1.setString("1: Level 1"); t1.setPosition(300, 200);
    t2.setFont(font); t2.setString("2: Level 2"); t2.setPosition(300, 300);
    t3.setFont(font); t3.setString("3: Level 3"); t3.setPosition(300, 400);
    inMenu = true;
    selectedLevel = 0;
}

void Menu::handleInput() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) { selectedLevel = 1; inMenu = false; }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) { selectedLevel = 2; inMenu = false; }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) { selectedLevel = 3; inMenu = false; }
}

void Menu::draw(sf::RenderWindow& win) {
    win.draw(t1);
    win.draw(t2);
    win.draw(t3);
}
