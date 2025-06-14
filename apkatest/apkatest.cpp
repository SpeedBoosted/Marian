#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "klasy.h"

using namespace sf;

int main() {
    RenderWindow window({ 800,600 }, "Marian");
    window.setFramerateLimit(60);

    // --- Audio ---
    SoundBuffer jumpBuf, shootBuf, enemyShootBuf;
    if (!jumpBuf.loadFromFile("jump.wav"))      return -1;
    if (!shootBuf.loadFromFile("shoot.wav"))    return -1;
    if (!enemyShootBuf.loadFromFile("shoot_2.wav")) return -1;

    Sound jumpSound(jumpBuf), shootSound(shootBuf), enemyShootSound(enemyShootBuf);

    // --- Fonty i teksty końcowe ---
    Font font;
    font.loadFromFile("arial.ttf");

    Text gameOverText("Game Over!\nPress any key...", font, 40);
    gameOverText.setFillColor(Color::Red);
    gameOverText.setPosition(200, 250);

    Text winText("You Win!\nPress any key...", font, 40);
    winText.setFillColor(Color::Yellow);
    winText.setPosition(200, 250);

    // --- Stan gry ---
    Menu                menu;
    Player              player;
    std::vector<Platform> platforms;
    std::vector<Enemy>    enemies;

    bool isGameOver = false;
    bool isWin = false;
    int  currentLevel = 0;

    // --- Lambda do wczytywania poziomów ---
    auto loadLevel = [&](int lvl) {
        platforms.clear();
        enemies.clear();
        switch (lvl) {
        case 1:
            player = Player(100, 500);
            platforms = {
                {0,550,800,50},
                {200,450,120,20},
                {400,350,120,20},
                {600,250,120,20}
            };
            enemies = { {220,410} };
            break;
        case 2:
            player = Player(50, 500);
            platforms = {
                {0,550,800,50},
                {150,450,150,20},
                {350,350,150,20},
                {550,250,150,20},
                {700,150,100,20}
            };
            enemies = { {180,410},{380,310} };
            break;
        case 3:
            player = Player(20, 500);
            platforms = {
                {0,550,800,50},
                {100,500,100,20},
                {300,400,100,20},
                {500,300,100,20},
                {700,200,100,20}
            };
            enemies = { {120,460},{320,360},{520,260} };
            break;
        }
        isGameOver = isWin = false;
        };

    // --- Główna pętla ---
    while (window.isOpen()) {
        Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == Event::Closed)
                window.close();

            // Restart po Game Over / Win
            if ((isGameOver || isWin) && ev.type == Event::KeyPressed) {
                menu.inMenu = true;
                currentLevel = 0;
                isGameOver = isWin = false;
            }

            // Skok (z obsługą dźwięku)
            if (!menu.inMenu && !isGameOver && !isWin
                && ev.type == Event::KeyPressed
                && ev.key.code == Keyboard::Space)
            {
                if (player.onGround) {
                    player.jump();
                    jumpSound.play();
                }
            }

            // Strzał gracza (myszką + dźwięk)
            if (!menu.inMenu && !isGameOver && !isWin
                && ev.type == Event::MouseButtonPressed
                && ev.mouseButton.button == Mouse::Left)
            {
                Vector2f target = window.mapPixelToCoords(
                    { ev.mouseButton.x, ev.mouseButton.y });
                player.shoot(target);
                shootSound.play();
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

        // Wczytaj poziom raz
        if (!menu.inMenu && currentLevel != menu.selectedLevel) {
            loadLevel(menu.selectedLevel);
            currentLevel = menu.selectedLevel;
        }

        // Sterowanie A/D
        if (!isGameOver && !isWin) {
            if (Keyboard::isKeyPressed(Keyboard::A)) player.move(-4.f);
            if (Keyboard::isKeyPressed(Keyboard::D)) player.move(4.f);
        }

        // --- Update gry ---
        if (!isGameOver && !isWin) {
            player.update(platforms);

            for (auto& e : enemies) {
                // teraz przekazujemy referencję do enemyShootSound
                e.update(platforms, player, enemyShootSound);

                // kolizje pocisków gracza → wróg
                for (auto& b : player.bullets) {
                    if (b.active && b.shape.getGlobalBounds()
                        .intersects(e.shape.getGlobalBounds()))
                    {
                        e.takeDamage(40);
                        b.active = false;
                    }
                }

                // kolizje pocisków wroga → gracz
                for (auto& b : e.bullets) {
                    if (b.active && b.shape.getGlobalBounds()
                        .intersects(player.shape.getGlobalBounds()))
                    {
                        player.takeDamage(10);
                        b.active = false;
                    }
                }
            }

            // Game Over?
            if (player.hp <= 0)
                isGameOver = true;

            // Przejście poziomu (prawa krawędź ekranu)
            if (player.shape.getPosition().x +
                player.shape.getSize().x >= 800.f)
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

        // --- Render ---
        window.clear(Color::Cyan);

        if (isGameOver) {
            window.draw(gameOverText);
        }
        else if (isWin) {
            window.draw(winText);
        }
        else {
            window.draw(player.shape);
            window.draw(player.hpBar);
            for (auto& p : platforms) window.draw(p.shape);
            for (auto& e : enemies)  window.draw(e.shape);
            for (auto& b : player.bullets)
                if (b.active) window.draw(b.shape);
            for (auto& e : enemies)
                for (auto& b : e.bullets)
                    if (b.active) window.draw(b.shape);
        }

        window.display();
    }

    return 0;
}
