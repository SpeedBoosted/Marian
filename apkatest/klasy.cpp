#include "klasy.h"

Platform::Platform(float x, float y, float width, float height) {
    shape.setPosition(x, y);
    shape.setSize(sf::Vector2f(width, height));
    shape.setFillColor(sf::Color::Green);
}

const float GRAVITY = 0.5f;
const float JUMP_SPEED = -10.0f;

Player::Player(float x, float y) {
    shape.setPosition(x, y);
    shape.setSize(sf::Vector2f(40, 60));
    shape.setFillColor(sf::Color::Red);
    velocity = sf::Vector2f(0, 0);
    onGround = false;
}

void Player::update(const std::vector<Platform>& platforms) {
    velocity.y += GRAVITY;
    shape.move(velocity);

    onGround = false;
    for (const auto& platform : platforms) {
        if (shape.getGlobalBounds().intersects(platform.shape.getGlobalBounds())) {
            if (velocity.y > 0) {
                shape.setPosition(shape.getPosition().x, platform.shape.getPosition().y - shape.getSize().y);
                velocity.y = 0;
                onGround = true;
            }
        }
    }
}

void Player::jump() {
    if (onGround) {
        velocity.y = JUMP_SPEED;
        onGround = false;
    }
}

void Player::move(float dx) {
    shape.move(dx, 0);
}

Enemy::Enemy(float x, float y) {
    shape.setPosition(x, y);
    shape.setSize(sf::Vector2f(40, 40));
    shape.setFillColor(sf::Color(139, 69, 19));
    speed = 2.0f;
    alive = true;
}

void Enemy::update(const std::vector<Platform>& platforms) {
    if (!alive) return;

    float verticalSpeed = 0.0f;
    verticalSpeed += GRAVITY;
    shape.move(speed, 0);
    bool onPlatform = false;         // czy jest na platformie
    for (const auto& platform : platforms) {
        sf::FloatRect enemyBounds = shape.getGlobalBounds();
        sf::FloatRect platformBounds = platform.shape.getGlobalBounds();
        if (
            enemyBounds.left + enemyBounds.width > platformBounds.left + 2 &&
            enemyBounds.left < platformBounds.left + platformBounds.width - 2 &&
            std::abs((enemyBounds.top + enemyBounds.height) - platformBounds.top) < 5.0f
            ) {
            onPlatform = true;
            shape.setPosition(shape.getPosition().x, platformBounds.top - enemyBounds.height);
            break;
        }
    }

    if (!onPlatform) {
        shape.move(0, verticalSpeed);
    }
    // Odbijanie od krawędzi platformy
    bool edge = true;
    for (const auto& platform : platforms) {
        sf::FloatRect enemyBounds = shape.getGlobalBounds();
        sf::FloatRect platformBounds = platform.shape.getGlobalBounds();
        // Jeśli Goomba jest na platformie i nie wychodzi poza jej krawędzie
        if (
            enemyBounds.left >= platformBounds.left - 1 &&
            enemyBounds.left + enemyBounds.width <= platformBounds.left + platformBounds.width + 1 &&
            std::abs((enemyBounds.top + enemyBounds.height) - platformBounds.top) < 5.0f
            ) {
            edge = false;
            break;
        }
    }
    if (edge) {
        speed = -speed;
        shape.move(speed * 2, 0); // odbicie na krawędzi
    }
}