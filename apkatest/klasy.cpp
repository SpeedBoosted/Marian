#include "klasy.h"
#include <cmath>

Platform::Platform(float x, float y, float width, float height) {
    shape.setSize(sf::Vector2f(width, height));
    shape.setPosition(x, y);
    shape.setFillColor(sf::Color::White);
}
void Platform::draw(sf::RenderWindow& window) {
    window.draw(shape);
}

Bullet::Bullet(float x, float y, float vx, float vy, bool enemy)
    : velocity(vx, vy), active(true), fromEnemy(enemy) {
    shape.setRadius(5);
    shape.setFillColor(enemy ? sf::Color::Red : sf::Color::Yellow);
    shape.setPosition(x, y);
}
void Bullet::update() {
    shape.move(velocity);
    if (shape.getPosition().x < 0 || shape.getPosition().x > 1280)
        active = false;
}
void Bullet::draw(sf::RenderWindow& window) {
    if (active) window.draw(shape);
}

Player::Player(float x, float y)
    : yVelocity(0), hp(100), onGround(false) {
    shape.setSize(sf::Vector2f(40, 40));
    shape.setFillColor(sf::Color::Green);
    shape.setPosition(x, y);
}
void Player::move(float dx, const std::vector<Platform>& platforms) {
    shape.move(dx, 0);
    for (const auto& p : platforms) {
        if (shape.getGlobalBounds().intersects(p.shape.getGlobalBounds())) {
            shape.move(-dx, 0);
            break;
        }
    }
}
bool Player::jump(const std::vector<Platform>& platforms) {
    if (onGround) {
        yVelocity = -12;
        onGround = false;
        return true;
    }
    return false;
}
void Player::update(const std::vector<Platform>& platforms) {
    yVelocity += 0.5f;
    shape.move(0, yVelocity);

    onGround = false;
    for (const auto& p : platforms) {
        if (shape.getGlobalBounds().intersects(p.shape.getGlobalBounds())) {
            shape.move(0, -yVelocity);
            yVelocity = 0;
            onGround = true;
            break;
        }
    }
}
Bullet Player::shoot() {
    return Bullet(shape.getPosition().x + 20, shape.getPosition().y + 20, 10, 0, false);
}
void Player::draw(sf::RenderWindow& window) {
    window.draw(shape);
}

Enemy::Enemy(float x, float y) {
    shape.setSize(sf::Vector2f(40, 40));
    shape.setFillColor(sf::Color::Red);
    shape.setPosition(x, y);
}
Bullet Enemy::shootTowards(const Player& player) {
    float dx = player.shape.getPosition().x - shape.getPosition().x;
    float dy = player.shape.getPosition().y - shape.getPosition().y;
    float len = std::sqrt(dx * dx + dy * dy);
    float vx = dx / len * 5;
    float vy = dy / len * 5;
    return Bullet(shape.getPosition().x + 20, shape.getPosition().y + 20, vx, vy, true);
}
void Enemy::draw(sf::RenderWindow& window) {
    window.draw(shape);
}
