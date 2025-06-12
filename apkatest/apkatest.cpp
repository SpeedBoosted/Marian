#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "klasy.h"
#include "levels.h"
#include <vector>
#include <iostream>

int main() {
    sf::RenderWindow window(sf::VideoMode(1280, 720), "2D Platformer");
    window.setFramerateLimit(60);

    sf::Font font;
    font.loadFromFile("arial.ttf");

    sf::Text menuText("Wybierz poziom:\n1. Poziom 1\n2. Poziom 2\n3. Poziom 3", font, 30);
    menuText.setPosition(100, 100);

    sf::Text gameOverText("GAME OVER", font, 50);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setPosition(500, 300);

    sf::Text victoryText("META OSIAGNIECIE", font, 50);
    victoryText.setFillColor(sf::Color::Green);
    victoryText.setPosition(450, 300);

    sf::RectangleShape hpBarBack(sf::Vector2f(200, 20));
    hpBarBack.setFillColor(sf::Color::Red);
    hpBarBack.setPosition(20, 20);

    sf::RectangleShape hpBar(sf::Vector2f(200, 20));
    hpBar.setFillColor(sf::Color::Green);
    hpBar.setPosition(20, 20);

    sf::SoundBuffer jumpBuffer, shootBuffer;
    jumpBuffer.loadFromFile("jump.wav");
    shootBuffer.loadFromFile("shoot.wav");

    sf::Sound jumpSound(jumpBuffer);
    sf::Sound shootSound(shootBuffer);

    int currentLevel = 0;
    bool inMenu = true;
    bool gameOver = false;
    bool levelCompleted = false;

    Player* player = nullptr;
    std::vector<Platform> platforms;
    std::vector<Enemy> enemies;
    std::vector<Bullet> bullets;
    sf::RectangleShape goal(sf::Vector2f(40, 40));
    goal.setFillColor(sf::Color::Cyan);

    auto loadLevel = [&](int levelNumber) {
        LevelData level;
        switch (levelNumber) {
        case 1: level = createLevel1(); break;
        case 2: level = createLevel2(); break;
        case 3: level = createLevel3(); break;
        default: return;
        }

        if (player) delete player;
        player = new Player(level.playerStart.x, level.playerStart.y);
        platforms = level.platforms;
        enemies = level.enemies;
        bullets.clear();
        goal.setPosition(level.goalPos);
        gameOver = false;
        levelCompleted = false;
        };

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (inMenu && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Num1) { currentLevel = 1; loadLevel(currentLevel); inMenu = false; }
                else if (event.key.code == sf::Keyboard::Num2) { currentLevel = 2; loadLevel(currentLevel); inMenu = false; }
                else if (event.key.code == sf::Keyboard::Num3) { currentLevel = 3; loadLevel(currentLevel); inMenu = false; }
            }
        }

        window.clear(sf::Color::Black);

        if (inMenu) {
            window.draw(menuText);
        }
        else if (gameOver) {
            window.draw(gameOverText);
        }
        else if (levelCompleted) {
            window.draw(victoryText);
        }
        else {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
                player->move(-5, platforms);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
                player->move(5, platforms);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                if (player->jump(platforms)) jumpSound.play();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
                bullets.push_back(player->shoot());
                shootSound.play();
            }

            player->update(platforms);

            // Update enemies
            for (auto& enemy : enemies) {
                Bullet b = enemy.shootTowards(*player);
                bullets.push_back(b);
            }

            // Update bullets
            for (auto& b : bullets)
                b.update();

            // Kolizje
            for (auto& b : bullets) {
                if (!b.active) continue;
                if (b.fromEnemy && player->shape.getGlobalBounds().intersects(b.shape.getGlobalBounds())) {
                    player->hp -= 10;
                    b.active = false;
                }
            }

            if (player->hp <= 0) {
                gameOver = true;
                continue;
            }

            if (player->shape.getGlobalBounds().intersects(goal.getGlobalBounds())) {
                levelCompleted = true;
                currentLevel++;
                if (currentLevel > 3) inMenu = true;
                else loadLevel(currentLevel);
            }

            // Rysowanie
            for (auto& p : platforms)
                p.draw(window);
            for (auto& e : enemies)
                e.draw(window);
            for (auto& b : bullets)
                b.draw(window);
            player->draw(window);
            window.draw(goal);

            hpBar.setSize(sf::Vector2f(2 * player->hp, 20));
            window.draw(hpBarBack);
            window.draw(hpBar);
        }

        window.display();
    }

    if (player) delete player;
    return 0;
}
