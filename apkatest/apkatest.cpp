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

int main() {
    std::srand((unsigned)std::time(nullptr));

    RenderWindow window(VideoMode(800, 600), "Marian");
    window.setFramerateLimit(60);
    View view(Vector2f(400.f, 300.f), Vector2f(800.f, 600.f));

    // tło poziomów
    Texture bgTex;
    if (!bgTex.loadFromFile("background.png")) std::cerr << "background.png missing\n";
    vector<Sprite> bgTiles;

    // dźwięki
    SoundBuffer jb, sb, pb, shb;
    jb.loadFromFile("jump.wav");
    sb.loadFromFile("shoot.wav");
    pb.loadFromFile("shoot_2.wav");
    shb.loadFromFile("shotgun.wav");
    Sound jumpSound(jb), shootSound(sb), pistolSound(pb), shotgunSound(shb);

    // teksty GameOver/YouWin
    Font font; font.loadFromFile("arial.ttf");
    Text gameOver("Game Over!\nPress any key...", font, 40);
    gameOver.setFillColor(Color::Red); gameOver.setPosition(200.f, 250.f);
    Text youWin("You Win!\nPress any key...", font, 40);
    youWin.setFillColor(Color::Yellow); youWin.setPosition(200.f, 250.f);

    // meta
    RectangleShape goal(Vector2f(40.f, 40.f));
    goal.setFillColor(Color::Yellow);

    // stan gry
    Menu menu;
    Player player;
    vector<Platform*> platforms;
    vector<Enemy> enemies;
    bool isGameOver = false, isWin = false;
    int currentLevel = 0;
    float levelWidth = 800.f;

    // loader poziomów
    auto loadLevel = [&](int lvl) {
        for (auto* p : platforms) delete p;
        platforms.clear();
        enemies.clear();

        switch (lvl) {
        case 1:
            levelWidth = 1600.f;
            player = Player(50.f, 500.f);
            platforms.push_back(new Platform(0, 550, 1600, 50));
            platforms.push_back(new MovingPlatform(300, 450, 120, 20, { 1.5f, 0 }, 100));
            platforms.push_back(new MovingPlatform(900, 350, 150, 20, { 0, 2 }, 80));
            platforms.push_back(new Platform(650, 300, 100, 20));
            platforms.push_back(new Platform(1200, 400, 200, 20));
            break;
        case 2:
            levelWidth = 2400.f;
            player = Player(50.f, 500.f);
            platforms.push_back(new Platform(0, 550, 2400, 50));
            platforms.push_back(new Platform(200, 450, 150, 20));
            platforms.push_back(new MovingPlatform(600, 350, 100, 20, { -1, 0 }, 150));
            platforms.push_back(new Platform(1100, 300, 120, 20));
            platforms.push_back(new MovingPlatform(1400, 200, 100, 20, { 0, 1.2f }, 120));
            platforms.push_back(new Platform(2000, 400, 150, 20));
            break;
        case 3:
            levelWidth = 3200.f;
            player = Player(50.f, 500.f);
            platforms.push_back(new Platform(0, 550, 3200, 50));
            for (int i = 0; i < 4; i++) {
                platforms.push_back(new Platform(300 + i * 400, 450 - i * 50, 120, 20));
                platforms.push_back(new MovingPlatform(
                    500 + i * 400, 350 - i * 30, 100, 20, { 0,1.5f }, 75));
            }
            break;
        }

        // kafelkowanie tła
        bgTiles.clear();
        int tw = bgTex.getSize().x, th = bgTex.getSize().y;
        float sy = 600.f / float(th), tileW = tw * sy;
        int cnt = int(std::ceil(levelWidth / tileW));
        for (int i = 0; i < cnt; i++) {
            Sprite s(bgTex);
            s.setScale(sy, sy);
            s.setPosition(i * tileW, 0);
            bgTiles.push_back(s);
        }

        // wrzucenie wrogów
        auto gb = platforms[0]->shape.getGlobalBounds();
        float ex = gb.left + (gb.width - 40.f) * (std::rand() / float(RAND_MAX));
        enemies.emplace_back(ex, gb.top - 40.f, Enemy::PISTOL);
        vector<Platform*> deck(platforms.begin() + 1, platforms.end());
        if (!deck.empty()) {
            auto pb2 = deck[std::rand() % deck.size()]->shape.getGlobalBounds();
            float px = pb2.left + (pb2.width - 40.f) * (std::rand() / float(RAND_MAX));
            enemies.emplace_back(px, pb2.top - 40.f, Enemy::SHOTGUN);
        }

        goal.setPosition(levelWidth - 60.f, 510.f);
        isGameOver = isWin = false;
        };

    // pętla główna
    while (window.isOpen()) {
        Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == Event::Closed) window.close();

            if ((isGameOver || isWin) && ev.type == Event::KeyPressed) {
                menu.inMenu = true; currentLevel = 0; isGameOver = isWin = false;
            }
            if (!menu.inMenu && !isGameOver && !isWin
                && ev.type == Event::KeyPressed && ev.key.code == Keyboard::Space
                && player.onGround) {
                player.jump(); jumpSound.play();
            }
            if (!menu.inMenu && !isGameOver && !isWin
                && ev.type == Event::MouseButtonPressed && ev.mouseButton.button == Mouse::Left) {
                Vector2f tgt = window.mapPixelToCoords(
                    Vector2i(ev.mouseButton.x, ev.mouseButton.y), view);
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
            for (auto& en : enemies) {
                Sound& snd = (en.type == Enemy::PISTOL ? pistolSound : shotgunSound);
                en.update(platforms, player, snd);

                // pociski gracza → wróg
                for (auto& b : player.bullets) {
                    if (b.active && b.shape.getGlobalBounds()
                        .intersects(en.shape.getGlobalBounds()))
                    {
                        en.takeDamage(40);
                        b.active = false;
                    }
                }
                // pociski wroga → gracz z pomniejszonym hitboxem
                for (auto& b : en.bullets) {
                    if (b.active && b.shape.getGlobalBounds()
                        .intersects(player.getCollisionBounds()))
                    {
                        player.takeDamage(10);
                        b.active = false;
                    }
                }
            }

            if (player.hp <= 0) isGameOver = true;
            if (player.shape.getGlobalBounds()
                .intersects(goal.getGlobalBounds()))
            {
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

        // rysowanie
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
            window.draw(goal);
            window.draw(player.shape);
            window.draw(player.hpBar);
            for (auto& en : enemies) window.draw(en.shape);
            for (auto& b : player.bullets) if (b.active) window.draw(b.shape);
            for (auto& en : enemies)
                for (auto& b : en.bullets) if (b.active) window.draw(b.shape);
        }
        window.display();
    }

    for (auto* p : platforms) delete p;
    return 0;
}
