#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "klasy.h"
#include <iostream>

using namespace sf;

int main() {
    RenderWindow window({ 800,600 }, "Marian");
    window.setFramerateLimit(60);

    // --- View (kamera) ---
    View view({ 0,0,800,600 });

    // --- Audio load ---
    SoundBuffer jumpBuf, pShootBuf, pistolBuf, shotgunBuf;
    bool haveJump = jumpBuf.loadFromFile("jump.wav");
    bool havePShoot = pShootBuf.loadFromFile("shoot.wav");
    bool havePistol = pistolBuf.loadFromFile("shoot_2.wav");
    bool haveShotgun = shotgunBuf.loadFromFile("shotgun.wav");
    if (!haveJump)    std::cerr << "Warning: jump.wav missing\n";
    if (!havePShoot)  std::cerr << "Warning: shoot.wav missing\n";
    if (!havePistol)  std::cerr << "Warning: shoot_2.wav missing\n";
    if (!haveShotgun) std::cerr << "Warning: shotgun.wav missing\n";

    Sound jumpSound, pShootSound, pistolSound, shotgunSound;
    if (haveJump)    jumpSound.setBuffer(jumpBuf);
    if (havePShoot)  pShootSound.setBuffer(pShootBuf);
    if (havePistol)  pistolSound.setBuffer(pistolBuf);
    if (haveShotgun) shotgunSound.setBuffer(shotgunBuf);

    // --- Font & Texts ---
    Font font;
    if (!font.loadFromFile("arial.ttf")) std::cerr << "Warning: arial.ttf missing\n";

    Text gameOverText("Game Over!\nPress any key...", font, 40);
    gameOverText.setFillColor(Color::Red);
    gameOverText.setPosition(200, 250);

    Text winText("You Win!\nPress any key...", font, 40);
    winText.setFillColor(Color::Yellow);
    winText.setPosition(200, 250);

    // --- Game state ---
    Menu                  menu;
    Player                player;
    std::vector<Platform*> platforms;
    std::vector<Enemy>    enemies;
    bool isGameOver = false, isWin = false;
    int  currentLevel = 0;
    float levelWidth = 800;

    // --- level loader ---
    auto loadLevel = [&](int lvl) {
        for (auto* p : platforms) delete p;
        platforms.clear();
        enemies.clear();

        switch (lvl) {
        case 1:
            levelWidth = 1600;  // 2 ekrany
            player = Player(50, 500);
            platforms.push_back(new Platform(0, 550, 1600, 50));
            platforms.push_back(new MovingPlatform(300, 450, 120, 20, { 1.5f,0 }, 100));
            platforms.push_back(new MovingPlatform(900, 350, 150, 20, { 0,2.f }, 80));
            platforms.push_back(new Platform(650, 300, 100, 20));
            platforms.push_back(new Platform(1200, 400, 200, 20));
            enemies.push_back({ 400,500,Enemy::PISTOL });
            break;
        case 2:
            levelWidth = 2400; // 3 ekrany
            player = Player(50, 500);
            platforms.push_back(new Platform(0, 550, 2400, 50));
            platforms.push_back(new Platform(200, 450, 150, 20));
            platforms.push_back(new MovingPlatform(600, 350, 100, 20, { -1.f,0 }, 150));
            platforms.push_back(new Platform(1100, 300, 120, 20));
            platforms.push_back(new MovingPlatform(1400, 200, 100, 20, { 0,1.2f }, 120));
            platforms.push_back(new Platform(2000, 400, 150, 20));
            enemies.push_back({ 300,500,Enemy::SHOTGUN });
            enemies.push_back({ 1300,350,Enemy::PISTOL });
            break;
        case 3:
            levelWidth = 3200; // 4 ekrany
            player = Player(50, 500);
            platforms.push_back(new Platform(0, 550, 3200, 50));
            for (int i = 0;i < 4;i++) {
                platforms.push_back(new Platform(300 + i * 400, 450 - 50 * i, 120, 20));
                platforms.push_back(new MovingPlatform(500 + i * 400, 350 - 30 * i, 100, 20, { 0,1.5f }, 75));
            }
            enemies.push_back({ 500,500,Enemy::SHOTGUN });
            enemies.push_back({ 1000,400,Enemy::PISTOL });
            enemies.push_back({ 1600,300,Enemy::SHOTGUN });
            break;
        }
        isGameOver = isWin = false;
        };

    // --- Main loop ---
    while (window.isOpen()) {
        Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == Event::Closed) window.close();

            // restart po końcu gry
            if ((isGameOver || isWin) && ev.type == Event::KeyPressed) {
                menu.inMenu = true;
                currentLevel = 0;
                isGameOver = isWin = false;
            }

            // skok + dźwięk
            if (!menu.inMenu && !isGameOver && !isWin
                && ev.type == Event::KeyPressed
                && ev.key.code == Keyboard::Space)
            {
                if (player.onGround) {
                    player.jump();
                    if (haveJump) jumpSound.play();
                }
            }

            // strzał gracza + dźwięk
            if (!menu.inMenu && !isGameOver && !isWin
                && ev.type == Event::MouseButtonPressed
                && ev.mouseButton.button == Mouse::Left)
            {
                Vector2f target = window.mapPixelToCoords(
                    { ev.mouseButton.x, ev.mouseButton.y }, view);
                player.shoot(target);
                if (havePShoot) pShootSound.play();
            }
        }

        // --- MENU ---
        if (menu.inMenu) {
            menu.handleInput();
            window.clear(Color::Black);
            menu.draw(window);
            window.display();
            continue;
        }

        // load once
        if (!menu.inMenu && currentLevel != menu.selectedLevel) {
            loadLevel(menu.selectedLevel);
            currentLevel = menu.selectedLevel;
        }

        // ruch A/D
        if (!isGameOver && !isWin) {
            if (Keyboard::isKeyPressed(Keyboard::A)) player.move(-4.f);
            if (Keyboard::isKeyPressed(Keyboard::D)) player.move(4.f);
        }

        // UPDATE gry
        if (!isGameOver && !isWin) {
            player.update(platforms);

            for (auto& e : enemies) {
                Sound& s = (e.type == Enemy::PISTOL ? pistolSound : shotgunSound);
                e.update(platforms, player, s);

                // pociski gracza → wróg
                for (auto& b : player.bullets) {
                    if (b.active && b.shape.getGlobalBounds()
                        .intersects(e.shape.getGlobalBounds()))
                    {
                        e.takeDamage(40);
                        b.active = false;
                    }
                }
                // pociski wroga → gracz
                for (auto& b : e.bullets) {
                    if (b.active && b.shape.getGlobalBounds()
                        .intersects(player.shape.getGlobalBounds()))
                    {
                        player.takeDamage(10);
                        b.active = false;
                    }
                }
            }

            if (player.hp <= 0) isGameOver = true;

            // przejście levelu
            if (player.shape.getPosition().x + player.shape.getSize().x >= levelWidth) {
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

        // kamera follow
        Vector2f c = player.shape.getPosition() + player.shape.getSize() / 2.f;
        c.x = std::clamp(c.x, 400.f, levelWidth - 400.f);
        view.setCenter(c);
        window.setView(view);

        // RENDER
        window.clear(Color::Cyan);
        if (isGameOver) {
            // rysujemy Game Over na domyślnym widoku
            window.setView(window.getDefaultView());
            window.draw(gameOverText);
        }
        else if (isWin) {
            window.setView(window.getDefaultView());
            window.draw(winText);
        }
        else {
            // gameplay
            for (auto* p : platforms) window.draw(p->shape);
            window.draw(player.shape);
            window.draw(player.hpBar);
            for (auto& e : enemies) window.draw(e.shape);
            for (auto& b : player.bullets) if (b.active) window.draw(b.shape);
            for (auto& e : enemies)
                for (auto& b : e.bullets) if (b.active) window.draw(b.shape);
        }
        window.display();
    }

    // cleanup
    for (auto* p : platforms) delete p;
    return 0;
}
