// main.cpp
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <algorithm>        // std::clamp
#include "klasy.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>

using namespace sf;
using std::vector;

int main()
{
    std::srand((unsigned)std::time(nullptr));
    RenderWindow window(VideoMode(800, 600), "Marian");
    window.setFramerateLimit(60);

    // główny widok kamery
    View view(Vector2f(400.f, 300.f), Vector2f(800.f, 600.f));

    // tło gry rozciągnięte na cały ekran
    Texture bgTex;
    if (!bgTex.loadFromFile("background.png"))
        std::cerr << "background.png missing\n";
    Sprite bgFull(bgTex);
    bgFull.setScale(
        float(window.getSize().x) / bgTex.getSize().x,
        float(window.getSize().y) / bgTex.getSize().y
    );
    bgFull.setPosition(0.f, 0.f);

    // dźwięki
    SoundBuffer jb, sb, pb, shb;
    jb.loadFromFile("jump.wav");
    sb.loadFromFile("shoot.wav");
    pb.loadFromFile("shoot_2.wav");
    shb.loadFromFile("shotgun.wav");
    Sound jumpSound(jb), shootSound(sb), pistolSound(pb), shotgunSound(shb);

    // font fallback
    Font font;
    if (!font.loadFromFile("arial.ttf"))
        std::cerr << "arial.ttf missing\n";

    // tekst Game Over (fallback)
    Text gameOverText("Game Over!\nPress any key...", font, 40);
    gameOverText.setFillColor(Color::Red);
    {
        FloatRect r = gameOverText.getLocalBounds();
        gameOverText.setOrigin(r.width / 2.f, r.height / 2.f);
        gameOverText.setPosition(400.f, 300.f);
    }

    // tekst You Win (fallback)
    Text youWinText("You Win!\nPress any key...", font, 40);
    youWinText.setFillColor(Color::Yellow);
    {
        FloatRect r = youWinText.getLocalBounds();
        youWinText.setOrigin(r.width / 2.f, r.height / 2.f);
        youWinText.setPosition(400.f, 300.f);
    }

    // sprite Game Over
    Texture goTex;
    Sprite  goSprite;
    bool    haveGOSprite = false;
    if (goTex.loadFromFile("game_over.png")) {
        haveGOSprite = true;
        goSprite.setTexture(goTex);
        float sx = float(window.getSize().x) / goTex.getSize().x;
        goSprite.setScale(sx, sx);
        goSprite.setOrigin(goTex.getSize().x / 2.f, goTex.getSize().y / 2.f);
        goSprite.setPosition(400.f, 300.f);
    }
    else {
        std::cerr << "game_over.png missing\n";
    }

    // sprite Win
    Texture winTex;
    Sprite  winSprite;
    bool    haveWinSprite = false;
    if (winTex.loadFromFile("win.png")) {
        haveWinSprite = true;
        winSprite.setTexture(winTex);
        float sx = float(window.getSize().x) / winTex.getSize().x;
        winSprite.setScale(sx, sx);
        winSprite.setOrigin(winTex.getSize().x / 2.f, winTex.getSize().y / 2.f);
        winSprite.setPosition(400.f, 300.f);
    }
    else {
        std::cerr << "win.png missing\n";
    }

    // cel poziomu (goal)
    RectangleShape goal(Vector2f(40.f, 40.f));
    goal.setFillColor(Color::Yellow);

    // obiekty gry
    Menu menu;
    Player player;
    vector<Platform*> platforms;
    vector<Enemy>    enemies;
    vector<Hazard>   hazards;

    bool isGameOver = false, isWin = false;
    int  currentLevel = 0;
    float levelWidth = 800.f;
    Clock levelClock;

    // loader poziomów
    auto loadLevel = [&](int lvl) {
        // usuń stare
        for (auto* p : platforms) delete p;
        platforms.clear();
        enemies.clear();
        hazards.clear();

        switch (lvl) {
        case 1:
            levelWidth = 1800.f;
            player = Player(50.f, 500.f);
            platforms.push_back(new Platform(0, 550, levelWidth, 50));
            platforms.push_back(new Platform(300, 450, 150, 20));
            platforms.push_back(new MovingPlatform(700, 350, 120, 20, { 1.2f,0 }, 120));
            platforms.push_back(new Platform(1100, 300, 100, 20));
            enemies.emplace_back(400, 510, Enemy::PISTOL);
            hazards.emplace_back(500, 530, 100, 20);
            break;
        case 2:
            levelWidth = 2400.f;
            player = Player(50.f, 500.f);
            platforms.push_back(new Platform(0, 550, levelWidth, 50));
            platforms.push_back(new Platform(200, 480, 100, 20));
            platforms.push_back(new MovingPlatform(500, 400, 120, 20, { -1.5f,0 }, 200));
            platforms.push_back(new MovingPlatform(900, 300, 100, 20, { 0,1.8f }, 150));
            platforms.push_back(new Platform(1400, 380, 150, 20));
            platforms.push_back(new Platform(1800, 310, 120, 20));
            enemies.emplace_back(600, 510, Enemy::PISTOL);
            enemies.emplace_back(1600, 270, Enemy::SHOTGUN);
            for (auto& e : enemies) {
                e.detectionRange = 550.f;
                e.shootCooldown = (e.type == Enemy::PISTOL ? 0.6f : 1.2f);
                e.speed *= 1.2f;
            }
            hazards.emplace_back(800, 530, 150, 20);
            hazards.emplace_back(1300, 530, 200, 20);
            break;
        case 3:
            levelWidth = 3200.f;
            player = Player(50.f, 500.f);
            platforms.push_back(new Platform(0, 550, levelWidth, 50));
            for (int i = 0; i < 6; ++i) {
                float x = 300.f + i * 450.f;
                float y = 450.f - (i % 2) * 80.f;
                platforms.push_back(new Platform(x, y, 120, 20));
                platforms.push_back(new MovingPlatform(x + 200, y - 100, 100, 20, { 0,2.5f }, 120));
            }
            platforms.push_back(new Platform(1500, 200, 50, 350));
            platforms.push_back(new Platform(2100, 350, 50, 200));
            for (int i = 0; i < 3; ++i)
                enemies.emplace_back(700 + 600 * i, 510 - (i % 2) * 200,
                    (i % 2 ? Enemy::SHOTGUN : Enemy::PISTOL));
            for (auto& e : enemies) {
                e.detectionRange = 650.f;
                e.shootCooldown = (e.type == Enemy::PISTOL ? 0.5f : 1.0f);
                e.speed *= 1.4f;
                e.retreatThreshold = 40;
                e.chaseThreshold = 70;
            }
            for (int i = 0; i < 4; ++i)
                hazards.emplace_back(900 + 500 * i, 530, 120, 20);
            break;
        }

        // ustaw pozycję celu
        goal.setPosition(levelWidth - 60.f, 510.f);

        isGameOver = false;
        isWin = false;
        levelClock.restart();
        };

    // główna pętla
    while (window.isOpen()) {
        Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == Event::Closed)
                window.close();

            // restart po GameOver/Win
            if ((isGameOver || isWin) && ev.type == Event::KeyPressed) {
                menu.inMenu = true;
                currentLevel = 0;
                isGameOver = isWin = false;
            }
            // skok
            if (!menu.inMenu && !isGameOver && !isWin
                && ev.type == Event::KeyPressed
                && ev.key.code == Keyboard::Space
                && player.onGround)
            {
                player.jump();
                jumpSound.play();
            }
            // strzał
            if (!menu.inMenu && !isGameOver && !isWin
                && ev.type == Event::MouseButtonPressed
                && ev.mouseButton.button == Mouse::Left)
            {
                Vector2f tgt = window.mapPixelToCoords(
                    Vector2i(ev.mouseButton.x, ev.mouseButton.y), view);
                player.shoot(tgt);
                shootSound.play();
            }
        }

        // update platform
        for (auto* p : platforms) p->update();

        // menu
        if (menu.inMenu) {
            menu.handleInput();
            window.clear(Color::Black);
            menu.draw(window);
            window.display();
            continue;
        }
        // load level
        if (currentLevel != menu.selectedLevel) {
            loadLevel(menu.selectedLevel);
            currentLevel = menu.selectedLevel;
        }

        // sterowanie graczem
        if (!isGameOver && !isWin) {
            if (Keyboard::isKeyPressed(Keyboard::A)) player.move(-4.f);
            if (Keyboard::isKeyPressed(Keyboard::D)) player.move(4.f);
        }

        // logika gry
        if (!isGameOver && !isWin) {
            player.update(platforms);
            for (auto& h : hazards) h.update(player);

            for (auto& e : enemies) {
                Sound& snd = (e.type == Enemy::PISTOL ? pistolSound : shotgunSound);
                e.update(platforms, player, snd);

                // trafienia gracz→wróg
                for (auto& b : player.bullets)
                    if (b.active && b.shape.getGlobalBounds()
                        .intersects(e.shape.getGlobalBounds()))
                    {
                        e.takeDamage(40);
                        b.active = false;
                    }
                // trafienia wróg→gracz
                for (auto& b : e.bullets)
                    if (b.active && b.shape.getGlobalBounds()
                        .intersects(player.getCollisionBounds()))
                    {
                        player.takeDamage(10);
                        b.active = false;
                    }
            }

            if (player.hp <= 0)
                isGameOver = true;

            if (levelClock.getElapsedTime().asSeconds() > 0.5f
                && player.shape.getGlobalBounds()
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

        // render
        window.clear();

        if (isGameOver) {
            window.setView(window.getDefaultView());
            window.draw(bgFull);
            if (haveGOSprite) window.draw(goSprite);
            else              window.draw(gameOverText);
        }
        else if (isWin) {
            window.setView(window.getDefaultView());
            window.draw(bgFull);
            if (haveWinSprite) window.draw(winSprite);
            else               window.draw(youWinText);
        }
        else {
            // normalna rozgrywka
            window.setView(window.getDefaultView());
            window.draw(bgFull);
            window.setView(view);

            window.draw(goal);
            for (auto* p : platforms) window.draw(p->shape);
            window.draw(player.shape);
            window.draw(player.hpBar);
            for (auto& h : hazards)   window.draw(h.shape);
            for (auto& e : enemies)   window.draw(e.shape);
            for (auto& b : player.bullets)
                if (b.active) window.draw(b.shape);
            for (auto& e : enemies)
                for (auto& b : e.bullets)
                    if (b.active) window.draw(b.shape);
        }

        window.display();
    }

    // sprzątanie
    for (auto* p : platforms)
        delete p;
    return 0;
}
