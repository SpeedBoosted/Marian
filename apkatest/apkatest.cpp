// main.cpp
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "klasy.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>
#include <algorithm>

using namespace sf;
using std::vector;

int main()
{
    std::srand((unsigned)std::time(nullptr));
    RenderWindow window(VideoMode(800, 600), "Marian");
    window.setFramerateLimit(60);

    View view(Vector2f(400, 300), Vector2f(800, 600));

    // tło
    Texture bgTex;
    if (!bgTex.loadFromFile("background.png"))
        std::cerr << "background.png missing\n";
    vector<Sprite> bgTiles;

    // dźwięki
    SoundBuffer jb, sb, pb, shb;
    jb.loadFromFile("jump.wav");
    sb.loadFromFile("shoot.wav");
    pb.loadFromFile("shoot_2.wav");
    shb.loadFromFile("shotgun.wav");
    Sound jumpSound(jb), shootSound(sb), pistolSound(pb), shotgunSound(shb);

    // GameOver/YouWin
    Font font; font.loadFromFile("arial.ttf");
    Text gameOver("Game Over!\nPress any key...", font, 40);
    gameOver.setFillColor(Color::Red);
    gameOver.setPosition(200, 250);
    Text youWin("You Win!\nPress any key...", font, 40);
    youWin.setFillColor(Color::Yellow);
    youWin.setPosition(200, 250);

    // meta
    RectangleShape goal(Vector2f(40, 40));
    goal.setFillColor(Color::Yellow);

    // stan gry
    Menu menu;
    Player player;
    vector<Platform*> platforms;
    vector<Enemy>    enemies;
    vector<Hazard>   hazards;
    bool isGameOver = false, isWin = false;
    int  currentLevel = 0;
    float levelWidth = 800.f;

    // loader poziomów
    auto loadLevel = [&](int lvl)
        {
            for (auto* p : platforms) delete p;
            platforms.clear();
            enemies.clear();
            hazards.clear();

            switch (lvl)
            {
            case 1:
                levelWidth = 1800.f;
                player = Player(50, 500);
                platforms.push_back(new Platform(0, 550, levelWidth, 50));
                platforms.push_back(new Platform(300, 450, 150, 20));
                platforms.push_back(new MovingPlatform(700, 350, 120, 20, { 1.2f,0 }, 120));
                platforms.push_back(new Platform(1100, 300, 100, 20));
                enemies.emplace_back(400, 510, Enemy::PISTOL);
                hazards.emplace_back(500, 530, 100, 20);
                break;

            case 2:
                levelWidth = 2400.f;
                player = Player(50, 500);
                platforms.push_back(new Platform(0, 550, levelWidth, 50));
                platforms.push_back(new Platform(200, 480, 100, 20));
                platforms.push_back(new MovingPlatform(500, 400, 120, 20, { -1.5f,0 }, 200));
                platforms.push_back(new MovingPlatform(900, 300, 100, 20, { 0,1.8f }, 150));
                platforms.push_back(new Platform(1400, 380, 150, 20));
                platforms.push_back(new Platform(1800, 310, 120, 20));
                enemies.emplace_back(600, 510, Enemy::PISTOL);
                enemies.emplace_back(1600, 270, Enemy::SHOTGUN);
                for (auto& e : enemies) {
                    e.detectionRange = 550;
                    e.shootCooldown = (e.type == Enemy::PISTOL ? 0.6f : 1.2f);
                    e.speed *= 1.2f;
                }
                hazards.emplace_back(800, 530, 150, 20);
                hazards.emplace_back(1300, 530, 200, 20);
                break;

            case 3:
                levelWidth = 3200.f;
                player = Player(50, 500);
                platforms.push_back(new Platform(0, 550, levelWidth, 50));
                for (int i = 0;i < 6;++i) {
                    float x = 300 + 450 * i;
                    float y = 450 - (i % 2) * 80;
                    platforms.push_back(new Platform(x, y, 120, 20));
                    platforms.push_back(new MovingPlatform(x + 200, y - 100, 100, 20, { 0,2.5f }, 120));
                }
                platforms.push_back(new Platform(1500, 200, 50, 350));
                platforms.push_back(new Platform(2100, 350, 50, 200));
                for (int i = 0;i < 3;++i)
                    enemies.emplace_back(700 + 600 * i, 510 - (i % 2) * 200, (i % 2 ? Enemy::SHOTGUN : Enemy::PISTOL));
                for (auto& e : enemies) {
                    e.detectionRange = 650;
                    e.shootCooldown = (e.type == Enemy::PISTOL ? 0.5f : 1.0f);
                    e.speed *= 1.4f;
                    e.retreatThreshold = 40;
                    e.chaseThreshold = 70;
                }
                for (int i = 0;i < 4;++i)
                    hazards.emplace_back(900 + 500 * i, 530, 120, 20);
                break;
            }

            // kafelkowanie tła
            bgTiles.clear();
            int tw = bgTex.getSize().x, th = bgTex.getSize().y;
            float sy = 600.f / float(th), tileW = tw * sy;
            int cnt = int(std::ceil(levelWidth / tileW));
            for (int i = 0;i < cnt;i++) {
                Sprite s(bgTex);
                s.setScale(sy, sy);
                s.setPosition(i * tileW, 0);
                bgTiles.push_back(s);
            }

            goal.setPosition(levelWidth - 60, 510);
            isGameOver = isWin = false;
        };

    // pętla gry
    while (window.isOpen())
    {
        Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == Event::Closed) window.close();
            if ((isGameOver || isWin) && ev.type == Event::KeyPressed) {
                menu.inMenu = true; currentLevel = 0;
                isGameOver = isWin = false;
            }
            if (!menu.inMenu && !isGameOver && !isWin
                && ev.type == Event::KeyPressed
                && ev.key.code == Keyboard::Space
                && player.onGround) {
                player.jump();
                jumpSound.play();
            }
            if (!menu.inMenu && !isGameOver && !isWin
                && ev.type == Event::MouseButtonPressed
                && ev.mouseButton.button == Mouse::Left) {
                Vector2f tgt = window.mapPixelToCoords(
                    { ev.mouseButton.x,ev.mouseButton.y }, view);
                player.shoot(tgt);
                shootSound.play();
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
        if (currentLevel != menu.selectedLevel) {
            loadLevel(menu.selectedLevel);
            currentLevel = menu.selectedLevel;
        }

        if (!isGameOver && !isWin) {
            if (Keyboard::isKeyPressed(Keyboard::A)) player.move(-4.f);
            if (Keyboard::isKeyPressed(Keyboard::D)) player.move(4.f);
        }

        if (!isGameOver && !isWin) {
            player.update(platforms);

            // pułapki raniące co sekundę po 10 HP
            for (auto& h : hazards)
                h.update(player);

            for (auto& e : enemies) {
                Sound& snd = (e.type == Enemy::PISTOL ? pistolSound : shotgunSound);
                e.update(platforms, player, snd);

                for (auto& b : player.bullets)
                    if (b.active && b.shape.getGlobalBounds()
                        .intersects(e.shape.getGlobalBounds())) {
                        e.takeDamage(40); b.active = false;
                    }
                for (auto& b : e.bullets)
                    if (b.active && b.shape.getGlobalBounds()
                        .intersects(player.getCollisionBounds())) {
                        player.takeDamage(10); b.active = false;
                    }
            }

            if (player.hp <= 0) isGameOver = true;
            if (player.shape.getGlobalBounds().intersects(goal.getGlobalBounds())) {
                if (menu.selectedLevel < 3) {
                    menu.selectedLevel++;
                    loadLevel(menu.selectedLevel);
                    currentLevel = menu.selectedLevel;
                }
                else {
                    isWin = true;
                }
            }
        }

        // kamera
        Vector2f cam = player.shape.getPosition() + player.shape.getSize() / 2.f;
        cam.x = std::clamp(cam.x, 400.f, levelWidth - 400.f);
        view.setCenter(cam);
        window.setView(view);

        // render
        window.clear(Color::Cyan);
        if (isGameOver) {
            window.setView(window.getDefaultView());
            window.draw(gameOver);
        }
        else if (isWin) {
            window.setView(window.getDefaultView());
            window.draw(youWin);
        }
        else {
            for (auto& s : bgTiles) window.draw(s);
            for (auto* p : platforms) window.draw(p->shape);
            for (auto& h : hazards)   window.draw(h.shape);
            window.draw(goal);
            window.draw(player.shape);
            window.draw(player.hpBar);
            for (auto& e : enemies)  window.draw(e.shape);
            for (auto& b : player.bullets) if (b.active) window.draw(b.shape);
            for (auto& e : enemies)
                for (auto& b : e.bullets) if (b.active) window.draw(b.shape);
        }
        window.display();
    }

    for (auto* p : platforms) delete p;
    return 0;
}
