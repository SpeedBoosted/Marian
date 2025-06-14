// main.cpp
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "klasy.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>   // srand, rand
#include <ctime>     // time

using namespace sf;
using std::vector;

int main() {
    std::srand(unsigned(std::time(nullptr)));

    RenderWindow window({ 800, 600 }, "Marian");
    window.setFramerateLimit(60);

    View view({ 0.f, 0.f, 800.f, 600.f });

    Texture bgTex;
    if (!bgTex.loadFromFile("background.png"))
        std::cerr << "Warning: background.png missing\n";
    vector<Sprite> bgSprites;

    SoundBuffer jumpBuf, pShootBuf, pistolBuf, shotgunBuf;
    bool haveJump = jumpBuf.loadFromFile("jump.wav");
    bool havePShoot = pShootBuf.loadFromFile("shoot.wav");
    bool havePistol = pistolBuf.loadFromFile("shoot_2.wav");
    bool haveShotgun = shotgunBuf.loadFromFile("shotgun.wav");
    if (!haveJump)    std::cerr << "jump.wav missing\n";
    if (!havePShoot)  std::cerr << "shoot.wav missing\n";
    if (!havePistol)  std::cerr << "shoot_2.wav missing\n";
    if (!haveShotgun) std::cerr << "shotgun.wav missing\n";

    Sound jumpSound, pShootSound, pistolSound, shotgunSound;
    if (haveJump)    jumpSound.setBuffer(jumpBuf);
    if (havePShoot)  pShootSound.setBuffer(pShootBuf);
    if (havePistol)  pistolSound.setBuffer(pistolBuf);
    if (haveShotgun) shotgunSound.setBuffer(shotgunBuf);

    Font font;
    if (!font.loadFromFile("arial.ttf"))
        std::cerr << "arial.ttf missing\n";
    Text gameOverText("Game Over!\nPress any key...", font, 40);
    gameOverText.setFillColor(Color::Red);
    gameOverText.setPosition(200.f, 250.f);
    Text winText("You Win!\nPress any key...", font, 40);
    winText.setFillColor(Color::Yellow);
    winText.setPosition(200.f, 250.f);

    RectangleShape goalShape({ 40.f, 40.f });
    goalShape.setFillColor(Color::Yellow);

    Menu menu;
    Player player;
    vector<Platform*> platforms;
    vector<Enemy>     enemies;
    bool isGameOver = false, isWin = false;
    int  currentLevel = 0;
    float levelWidth = 800.f;

    auto loadLevel = [&](int lvl) {
        for (auto* p : platforms) delete p;
        platforms.clear();
        enemies.clear();

        switch (lvl) {
        case 1:
            levelWidth = 1600.f;
            player = Player(50.f, 500.f);
            platforms.push_back(new Platform(0.f, 550.f, 1600.f, 50.f));
            platforms.push_back(new MovingPlatform(300.f, 450.f, 120.f, 20.f, { 1.5f,0.f }, 100.f));
            platforms.push_back(new MovingPlatform(900.f, 350.f, 150.f, 20.f, { 0.f,2.f }, 80.f));
            platforms.push_back(new Platform(650.f, 300.f, 100.f, 20.f));
            platforms.push_back(new Platform(1200.f, 400.f, 200.f, 20.f));
            break;
        case 2:
            levelWidth = 2400.f;
            player = Player(50.f, 500.f);
            platforms.push_back(new Platform(0.f, 550.f, 2400.f, 50.f));
            platforms.push_back(new Platform(200.f, 450.f, 150.f, 20.f));
            platforms.push_back(new MovingPlatform(600.f, 350.f, 100.f, 20.f, { -1.f,0.f }, 150.f));
            platforms.push_back(new Platform(1100.f, 300.f, 120.f, 20.f));
            platforms.push_back(new MovingPlatform(1400.f, 200.f, 100.f, 20.f, { 0.f,1.2f }, 120.f));
            platforms.push_back(new Platform(2000.f, 400.f, 150.f, 20.f));
            break;
        case 3:
            levelWidth = 3200.f;
            player = Player(50.f, 500.f);
            platforms.push_back(new Platform(0.f, 550.f, 3200.f, 50.f));
            for (int i = 0; i < 4; ++i) {
                platforms.push_back(new Platform(
                    300.f + i * 400.f,
                    450.f - i * 50.f,
                    120.f, 20.f));
                platforms.push_back(new MovingPlatform(
                    500.f + i * 400.f,
                    350.f - i * 30.f,
                    100.f, 20.f,
                    { 0.f,1.5f }, 75.f));
            }
            break;
        }

        // tło
        bgSprites.clear();
        int texW = bgTex.getSize().x, texH = bgTex.getSize().y;
        float scaleY = 600.f / texH, tileW = texW * scaleY;
        int countX = int(std::ceil(levelWidth / tileW));
        for (int i = 0; i < countX; ++i) {
            Sprite s(bgTex);
            s.setScale(scaleY, scaleY);
            s.setPosition(i * tileW, 0.f);
            bgSprites.push_back(s);
        }

        // wrogowie – tylko 1 na ziemi
        auto groundB = platforms[0]->shape.getGlobalBounds();
        float gx = groundB.left + (groundB.width - 40.f) * (std::rand() / float(RAND_MAX));
        enemies.emplace_back(gx, groundB.top - 40.f, Enemy::PISTOL);

        // i 1 na losowej platformie (jeśli jest)
        vector<Platform*> deck(platforms.begin() + 1, platforms.end());
        if (!deck.empty()) {
            auto p = deck[std::rand() % deck.size()]->shape.getGlobalBounds();
            float px = p.left + (p.width - 40.f) * (std::rand() / float(RAND_MAX));
            enemies.emplace_back(px, p.top - 40.f, Enemy::SHOTGUN);
        }

        goalShape.setPosition(levelWidth - 60.f, 550.f - 40.f);
        isGameOver = isWin = false;
        };

    while (window.isOpen()) {
        Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == Event::Closed) window.close();
            if ((isGameOver || isWin) && ev.type == Event::KeyPressed) {
                menu.inMenu = true;
                currentLevel = 0;
                isGameOver = isWin = false;
            }
            if (!menu.inMenu && !isGameOver && !isWin &&
                ev.type == Event::KeyPressed &&
                ev.key.code == Keyboard::Space &&
                player.onGround)
            {
                player.jump();
                if (haveJump) jumpSound.play();
            }
            if (!menu.inMenu && !isGameOver && !isWin &&
                ev.type == Event::MouseButtonPressed &&
                ev.mouseButton.button == Mouse::Left)
            {
                Vector2f tgt = window.mapPixelToCoords(
                    { ev.mouseButton.x, ev.mouseButton.y }, view);
                player.shoot(tgt);
                if (havePShoot) pShootSound.play();
            }
        }

        for (auto* p : platforms) p->update();

        if (menu.inMenu) {
            menu.handleInput();
            window.clear(Color::Black);
            menu.draw(window);
            window.display();
            continue;
        }
        if (!menu.inMenu && currentLevel != menu.selectedLevel) {
            loadLevel(menu.selectedLevel);
            currentLevel = menu.selectedLevel;
        }

        if (!isGameOver && !isWin) {
            if (Keyboard::isKeyPressed(Keyboard::A)) player.move(-4.f);
            if (Keyboard::isKeyPressed(Keyboard::D)) player.move(4.f);
        }

        if (!isGameOver && !isWin) {
            player.update(platforms);
            for (auto& e : enemies) {
                Sound& s = (e.type == Enemy::PISTOL ? pistolSound : shotgunSound);
                e.update(platforms, player, s);
                for (auto& b : player.bullets)
                    if (b.active && b.shape.getGlobalBounds()
                        .intersects(e.shape.getGlobalBounds()))
                    {
                        e.takeDamage(40); b.active = false;
                    }
                for (auto& b : e.bullets)
                    if (b.active && b.shape.getGlobalBounds()
                        .intersects(player.shape.getGlobalBounds()))
                    {
                        player.takeDamage(10); b.active = false;
                    }
            }
            if (player.hp <= 0) isGameOver = true;
            if (player.shape.getGlobalBounds()
                .intersects(goalShape.getGlobalBounds()))
            {
                if (menu.selectedLevel < 3) {
                    menu.selectedLevel++;
                    loadLevel(menu.selectedLevel);
                    currentLevel = menu.selectedLevel;
                }
                else isWin = true;
            }
        }

        Vector2f c = player.shape.getPosition() + player.shape.getSize() / 2.f;
        c.x = std::clamp(c.x, 400.f, levelWidth - 400.f);
        view.setCenter(c);
        window.setView(view);

        window.clear(Color::Cyan);
        if (isGameOver) {
            window.setView(window.getDefaultView());
            window.draw(gameOverText);
        }
        else if (isWin) {
            window.setView(window.getDefaultView());
            window.draw(winText);
        }
        else {
            for (auto& s : bgSprites)     window.draw(s);
            for (auto* p : platforms)     window.draw(p->shape);
            window.draw(goalShape);
            window.draw(player.shape);
            window.draw(player.hpBar);
            for (auto& e : enemies)       window.draw(e.shape);
            for (auto& b : player.bullets) if (b.active) window.draw(b.shape);
            for (auto& e : enemies)
                for (auto& b : e.bullets)    if (b.active) window.draw(b.shape);
        }
        window.display();
    }

    for (auto* p : platforms) delete p;
    return 0;
}
