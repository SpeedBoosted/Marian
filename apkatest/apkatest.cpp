#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "klasy.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <functional>

using namespace sf;
using std::vector;

int main()
{
    std::srand((unsigned)std::time(nullptr));

    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;

    RenderWindow window(VideoMode(800, 600), "Marian", sf::Style::Default, settings);
    window.setFramerateLimit(60);

    View view(Vector2f(400.f, 300.f), Vector2f(800.f, 600.f));

    // Tło
    Texture bgTex;
    if (!bgTex.loadFromFile("background.png"))
        std::cerr << "background.png missing\n";
    vector<Sprite> bgTiles;

    std::vector<Alcohol> drinks;

    // Dźwięki
    SoundBuffer jb, sb, pb, shb, bb;
    jb.loadFromFile("jump.wav");
    sb.loadFromFile("shoot.wav");
    pb.loadFromFile("shoot_2.wav");
    shb.loadFromFile("shotgun.wav");
    bb.loadFromFile("boss1.wav");
    Sound jumpSound(jb), shootSound(sb), pistolSound(pb), shotgunSound(shb), bossSound(bb);
    sf::Music menuMusic, level1Music, level2Music, level3Music;
    if (!menuMusic.openFromFile("menumuza.wav"))
        std::cerr << "menumuza.wav missing\n";
    if (!level1Music.openFromFile("level1.wav"))
        std::cerr << "level1.wav missing\n";
    if (!level2Music.openFromFile("level2.wav"))
        std::cerr << "level2.wav missing\n";
    if (!level3Music.openFromFile("level3.wav"))
        std::cerr << "level3.wav missing\n";
    menuMusic.setLoop(true);
    level1Music.setLoop(true);
    level2Music.setLoop(true);
    level3Music.setLoop(true);

    std::function<void(int)> loadLevel;

    // Ekran GameOver
    Font font; font.loadFromFile("arial.ttf");
    Text gameOverText("Rip BOZO!\nPress any key...", font, 40);
    gameOverText.setFillColor(Color::Red);
    gameOverText.setPosition(200.f, 250.f);

    Texture gameOverTex;
    if (!gameOverTex.loadFromFile("game_over.png"))
        std::cerr << "game_over.png missing\n";
    Sprite gameOverSprite(gameOverTex);
    gameOverSprite.setOrigin(gameOverTex.getSize().x / 2.f, gameOverTex.getSize().y / 2.f);
    gameOverSprite.setPosition(400.f, 300.f);

    // Napisy końcowe
    sf::Music endingMusic;
    if (!endingMusic.openFromFile("ending.wav"))
        std::cerr << "ending.wav missing\n";
    endingMusic.setLoop(false);
    Texture logoTex;
    if (!logoTex.loadFromFile("logo.png"))
        std::cerr << "logo.png missing\n";
    Sprite logoSprite(logoTex);
    logoSprite.setOrigin(logoTex.getSize().x / 2.f, logoTex.getSize().y / 2.f);
    logoSprite.setPosition(400.f, 120.f);
    bool inCredits = false;
    float creditsY = 600.f;
    const float creditsSpeed = 50.f;
    std::vector<std::string> creditsLines = {
        "Autorzy:","Jan Dymek","Mikolaj Fraszczak",
        "",
        "W rolach glownych","Marian - Martin Yan","Przeciwnik 1 - Jason Mamoa","Przeciwnik 2 - Jan Dymek",
        "",
        "Muzyka:",
        "Elevator - Kevin MacLeod",
        "Spazzmatica Polka - Kevin MacLeod",
        "Spazzmatica Polka - Kevin MacLeod",
        "Spazzmatica Polka - Kevin MacLeod",
        "Nothings Gonna Stop Us Now - Greg O'Connor",
        "",
        "Sponsorzy:","Twoja Stara Foundation",
        "",
        "Dziekujemy za gre!",
        "",
        "2024"
    };
    Text creditsText("", font, 32);
    creditsText.setFillColor(Color::White);
    sf::Clock frameClock;
    Texture doorTex;
    if (!doorTex.loadFromFile("door.png"))
        std::cerr << "door.png missing\n";
    Sprite doorSprite;

    // Stan gry
    Menu menu;
    Player player;
    vector<Platform*> platforms;
    vector<Enemy>    enemies;
    vector<Hazard>    hazards;
    bool isGameOver = false, isWin = false;
    int  currentLevel = 0;
    float levelWidth = 800.f;
    RectangleShape giantWall;
    bool wallActive = false;
    sf::Clock wallClock;
    float wallSpeed = 0.f;
    // Lambda funkcja do odtwarzania muzyki
    auto playMusic = [&](sf::Music& music) {
        menuMusic.stop();
        level1Music.stop();
        level2Music.stop();
        level3Music.stop();
        music.play();
        };
    // Lambda funkcja do ładowania poziomów
    loadLevel = [&](int lvl)
        {
            // Czyszczenie poprzedniego poziomu
            for (auto* p : platforms) delete p;
            platforms.clear();
            enemies.clear();
            hazards.clear();
            drinks.clear();

            // Definicje poziomów
            switch (lvl)
            {
            case 1:
                levelWidth = 1800.f;
                player = Player(50.f, 500.f);
                player.resetAlcoholEffects();
                platforms.push_back(new Platform(0.f, 550.f, levelWidth, 50.f));
                platforms.push_back(new Platform(300.f, 450.f, 150.f, 20.f));
                platforms.push_back(new MovingPlatform(700.f, 350.f, 120.f, 20.f, { 1.2f,0.f }, 120.f));
                platforms.push_back(new Platform(1100.f, 300.f, 100.f, 20.f));
                enemies.emplace_back(400.f, 550.f, Enemy::PISTOL);
                hazards.emplace_back(500.f, 530.f, 100.f, 20.f);
                drinks.emplace_back(Alcohol(AlcoholType::Piwo, 200.f, 520.f));
                drinks.emplace_back(Alcohol(AlcoholType::Wodka, 250.f, 520.f));
                drinks.emplace_back(Alcohol(AlcoholType::Kubus, 300.f, 520.f));
                drinks.emplace_back(Alcohol(AlcoholType::Wino, 350.f, 520.f));
                std::cout << "Spawned " << drinks.size() << " drinks for level 1\n";
                playMusic(level1Music);
                wallActive = false;
                break;

            case 2:
                levelWidth = 2400.f;
                player = Player(50.f, 500.f);
                platforms.push_back(new Platform(0.f, 550.f, levelWidth, 50.f));
                platforms.push_back(new Platform(200.f, 480.f, 100.f, 20.f));
                platforms.push_back(new MovingPlatform(500.f, 400.f, 120.f, 20.f, { -1.5f,0.f }, 200.f));
                platforms.push_back(new MovingPlatform(900.f, 300.f, 100.f, 20.f, { 0.f,1.8f }, 150.f));
                platforms.push_back(new Platform(1400.f, 380.f, 150.f, 20.f));
                platforms.push_back(new Platform(1800.f, 310.f, 120.f, 20.f));
                enemies.emplace_back(600.f, 550.f, Enemy::PISTOL);
                enemies.emplace_back(1800.f + 120.f / 2.f, 310.f, Enemy::SHOTGUN);
                for (auto& e : enemies) {
                    e.detectionRange = 550.f;
                    e.shootCooldown = (e.type == Enemy::PISTOL ? 0.6f : 1.2f);
                    e.speed *= 1.2f;
                }
                hazards.emplace_back(800.f, 530.f, 150.f, 20.f);
                hazards.emplace_back(1300.f, 530.f, 200.f, 20.f);
                playMusic(level2Music);
                wallActive = false;
                break;

            case 3:
                levelWidth = 3200.f;
                player = Player(50.f, 500.f);
                platforms.push_back(new Platform(0.f, 550.f, levelWidth, 50.f));
                for (int i = 0; i < 6; ++i) {
                    float x = 300.f + 450.f * i;
                    float y = 450.f - (i % 2) * 80.f;
                    platforms.push_back(new Platform(x, y, 120.f, 20.f));
                    platforms.push_back(new MovingPlatform(x + 200.f, y - 100.f, 100.f, 20.f, { 0.f,2.5f }, 120.f));
                }
                platforms.push_back(new Platform(1500.f, 200.f, 50.f, 350.f));
                platforms.push_back(new Platform(2100.f, 350.f, 50.f, 200.f));
                for (int i = 0; i < 3; ++i)
                    enemies.emplace_back(700.f + 600.f * i, 550.f - (i % 2) * 200.f, (i % 2 ? Enemy::SHOTGUN : Enemy::PISTOL));
                for (auto& e : enemies) {
                    e.detectionRange = 650.f;
                    e.shootCooldown = (e.type == Enemy::PISTOL ? 0.5f : 1.0f);
                    e.speed *= 1.4f;
                    e.retreatThreshold = 40;
                    e.chaseThreshold = 70;
                }
                for (int i = 0; i < 4; ++i)
                    hazards.emplace_back(900.f + 500.f * i, 530.f, 120.f, 20.f);
                playMusic(level1Music);
                wallActive = false;
                break;

            case 4: {
                levelWidth = 1600.f;
                player = Player(50.f, 500.f);
                platforms.push_back(new Platform(0.f, 550.f, 300.f, 50.f));
                platforms.push_back(new Platform(500.f, 550.f, 300.f, 50.f));
                platforms.push_back(new Platform(900.f, 550.f, 250.f, 50.f));
                platforms.push_back(new Platform(1250.f, 550.f, 350.f, 50.f));
                platforms.push_back(new Platform(300.f, 450.f, 100.f, 20.f));
                platforms.push_back(new Platform(650.f, 400.f, 100.f, 20.f));
                platforms.push_back(new Platform(1000.f, 450.f, 100.f, 20.f));
                enemies.emplace_back(600.f, 550.f, Enemy::SHOTGUN);
                enemies.emplace_back(950.f, 550.f, Enemy::SHOTGUN);
                float bossX = 1400.f;
                float bossY = 550.f;
                enemies.emplace_back(bossX, bossY, Enemy::BOSS);
                playMusic(level2Music);
                wallActive = false;
                break;
            }
            case 5: {
                levelWidth = 2500.f;
                player = Player(50.f, 500.f);
                giantWall.setSize(Vector2f(60.f, 600.f));
                giantWall.setFillColor(Color(120, 120, 120));
                giantWall.setPosition(0.f, 0.f);
                wallActive = true;
                wallSpeed = 200.f;
                wallClock.restart();
                platforms.push_back(new Platform(0.f, 550.f, levelWidth, 50.f));
                platforms.push_back(new Platform(400.f, 480.f, 150.f, 20.f));
                platforms.push_back(new MovingPlatform(700.f, 400.f, 120.f, 20.f, { 0.f, -1.5f }, 100.f));
                platforms.push_back(new Platform(850.f, 300.f, 30.f, 250.f));
                platforms.push_back(new Platform(1100.f, 470.f, 180.f, 20.f));
                platforms.push_back(new MovingPlatform(1400.f, 370.f, 100.f, 20.f, { -2.0f, 0.f }, 200.f));
                platforms.push_back(new Platform(1700.f, 320.f, 150.f, 20.f));
                platforms.push_back(new Platform(1650.f, 250.f, 30.f, 300.f));
                platforms.push_back(new MovingPlatform(1950.f, 250.f, 100.f, 20.f, { 0.f, 2.0f }, 120.f));
                platforms.push_back(new Platform(2150.f, 350.f, 120.f, 20.f));
                platforms.push_back(new Platform(2350.f, 450.f, 100.f, 20.f));
                platforms.push_back(new Platform(2250.f, 220.f, 30.f, 330.f));
                playMusic(level3Music);
                break;
            }

            }

            // Kafelkowanie tła
            bgTiles.clear();
            int tw = bgTex.getSize().x, th = bgTex.getSize().y;
            float sy = (600.f / static_cast<float>(th)) * 2.0f;
            float tileW = static_cast<float>(tw) * sy;
            int cnt = static_cast<int>(std::ceil(levelWidth / tileW));
            for (int i = 0; i < cnt; i++) {
                Sprite s(bgTex);
                s.setScale(sy, sy);
                s.setPosition(static_cast<float>(i) * tileW, 0.f);
                bgTiles.push_back(s);
            }

            // Ustawienie drzwi na końcu poziomu
            doorSprite.setTexture(doorTex);
            doorSprite.setScale(0.1f, 0.1f);
            float groundY = 510.f;
            Vector2f doorTexSize = Vector2f(
                static_cast<float>(doorTex.getSize().x) * doorSprite.getScale().x,
                static_cast<float>(doorTex.getSize().y) * doorSprite.getScale().y
            );
            doorSprite.setPosition(levelWidth - doorTexSize.x, groundY - doorTexSize.y);
            isGameOver = isWin = false;
        };


    // Główna pętla gry
    while (window.isOpen())
    {
        float deltaTime = frameClock.restart().asSeconds();
        // Obsługa napisów końcowych
        if (inCredits) {
            if (endingMusic.getStatus() != sf::Music::Playing)
                playMusic(endingMusic);

            window.setView(window.getDefaultView());
            window.clear(Color::Black);
            std::string allLines;
            for (const auto& line : creditsLines)
                allLines += line + "\n";
            creditsText.setString(allLines);
            float logoY = creditsY;
            logoSprite.setPosition(400.f, logoY + static_cast<float>(logoTex.getSize().y) / 2.f);
            float textStartY = logoY + static_cast<float>(logoTex.getSize().y) + 20.f;
            creditsText.setPosition(
                400.f - creditsText.getLocalBounds().width / 2.f,
                textStartY
            );
            creditsY -= creditsSpeed * deltaTime;

            window.draw(logoSprite);
            window.draw(creditsText);
            window.display();

            if (textStartY + creditsText.getLocalBounds().height < 0.f) {
                inCredits = false;
                menu.inMenu = true;
                playMusic(menuMusic);
            }
            continue;
        }
        Event ev;
        // Obsługa zdarzeń
        while (window.pollEvent(ev)) {
            if (ev.type == Event::Closed) window.close();
            if (isGameOver && ev.type == Event::KeyPressed) {
                menu.inMenu = true; currentLevel = 0;
                isGameOver = false;
            }
            if (isWin && ev.type == Event::KeyPressed) {
                isWin = false;
                inCredits = true;
                creditsY = 600.f;
                playMusic(endingMusic);
            }
            if (inCredits && ev.type == Event::KeyPressed) {
                inCredits = false;
                menu.inMenu = true;
                playMusic(menuMusic);
            }
            // Skok gracza
            if (!menu.inMenu && !isGameOver && !isWin
                && ev.type == Event::KeyPressed
                && ev.key.code == Keyboard::Space
                && player.onGround) {
                player.jump();
                jumpSound.play();
            }
            // Strzał gracza
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

        // Logika menu
        if (menu.inMenu) {
            if (menuMusic.getStatus() != sf::Music::Playing)
                playMusic(menuMusic);
            menu.handleInput();
            window.clear(Color::Black);
            menu.draw(window);
            window.display();
            continue;
        }
        // Ładowanie nowego poziomu
        if (currentLevel != menu.selectedLevel) {
            loadLevel(menu.selectedLevel);
            currentLevel = menu.selectedLevel;
        }

        // Logika gry
        if (!isGameOver && !isWin) {
            if (Keyboard::isKeyPressed(Keyboard::A)) player.move(-4.f * player.speedBoost);
            if (Keyboard::isKeyPressed(Keyboard::D)) player.move(4.f * player.speedBoost);
            player.pickUpAlcohol(drinks);

            // Wybór potka
            if (Keyboard::isKeyPressed(Keyboard::Num1)) player.selectedAlcohol = AlcoholType::Piwo;
            if (Keyboard::isKeyPressed(Keyboard::Num2)) player.selectedAlcohol = AlcoholType::Wodka;
            if (Keyboard::isKeyPressed(Keyboard::Num3)) player.selectedAlcohol = AlcoholType::Kubus;
            if (Keyboard::isKeyPressed(Keyboard::Num4)) player.selectedAlcohol = AlcoholType::Wino;

            if (Keyboard::isKeyPressed(Keyboard::G))
                player.useAlcohol();
            player.update(platforms);

            // Aktualizacja pułapek
            for (auto& h : hazards)
                h.update(player);

            // Logika ruchomej ściany (dla poziomu 5)
            if (wallActive && wallClock.getElapsedTime().asSeconds() >= 2.f) {

                if (giantWall.getGlobalBounds().intersects(player.getCollisionBounds())) {
                    player.hp = 0;
                    isGameOver = true;
                }
                Vector2f pos = giantWall.getPosition();
                pos.x += wallSpeed * deltaTime;
                if (pos.x + giantWall.getSize().x > levelWidth)
                    pos.x = levelWidth - giantWall.getSize().x;
                giantWall.setPosition(pos);
            }
            // Aktualizacja wrogów
            for (auto& e : enemies) {
                if (!e.alive) continue;
                Sound& snd = (e.type == Enemy::PISTOL ? pistolSound : (e.type == Enemy::SHOTGUN ? shotgunSound : bossSound));
                e.update(platforms, player, snd);

                // Kolizje pocisków gracza z wrogami
                for (auto& b : player.bullets)
                    if (b.active && b.shape.getGlobalBounds().intersects(e.shape.getGlobalBounds())) {
                        if (e.type == Enemy::BOSS) {
                            e.hits++;
                            if (e.hits >= 5) e.alive = false;
                        }
                        else {
                            e.takeDamage(40);
                        }
                        b.active = false;
                    }

                // Kolizje pocisków wroga z graczem
                for (auto& b : e.bullets)
                    if (b.active && b.shape.getGlobalBounds().intersects(player.getCollisionBounds())) {
                        player.takeDamage(10);
                        b.active = false;
                    }
            }

            if (player.hp <= 0) isGameOver = true;
            // Kolizja gracza z drzwiami (ukończenie poziomu)
            if (player.getCollisionBounds().intersects(doorSprite.getGlobalBounds())) {
                bool bossAlive = false;
                if (menu.selectedLevel == 4) {
                    for (const auto& e : enemies) {
                        if (e.type == Enemy::BOSS && e.alive) {
                            bossAlive = true;
                            break;
                        }
                    }
                }
                if (menu.selectedLevel == 4 && bossAlive) {
                    // Nie pozwól ukończyć poziomu 4, dopóki boss żyje
                }
                else if (menu.selectedLevel < 5) {
                    menu.selectedLevel++;
                    loadLevel(menu.selectedLevel);
                    currentLevel = menu.selectedLevel;
                }
                else {
                    inCredits = true;
                    creditsY = 600.f;
                    playMusic(endingMusic);
                }
            }
        }

        // Aktualizacja kamery
        Vector2f cam = player.shape.getPosition() + player.shape.getSize() / 2.f;
        cam.x = std::max(400.f, std::min(cam.x, levelWidth - 400.f));
        float bgVisibleHeight = static_cast<float>(bgTex.getSize().y) * ((600.f / static_cast<float>(bgTex.getSize().y)) * 2.0f);
        float minY = 300.f;
        float maxY = bgVisibleHeight - 600.f;
        cam.y = std::max(minY, std::min(cam.y, maxY));
        view.setCenter(cam);
        window.setView(view);

        // Wyświetlanie inwentarza potków
        sf::Text potionText;
        potionText.setFont(font);
        std::string names[4] = { "Piwo", "Wodka", "Kubus", "Wino" };
        potionText.setString(names[static_cast<int>(player.selectedAlcohol)] + ": " + std::to_string(player.alcoholInventory[player.selectedAlcohol]));
        potionText.setCharacterSize(24);
        potionText.setFillColor(sf::Color::White);
        potionText.setPosition(view.getCenter().x - 400.f + 10.f, view.getCenter().y - 300.f + 10.f);
        if (!menu.inMenu && !isGameOver && !inCredits)
            window.draw(potionText);

        // Renderowanie elementów gry
        window.clear(Color::Cyan);
        if (isGameOver) {
            window.setView(window.getDefaultView());
            window.draw(gameOverSprite);
            gameOverText.setPosition(window.getDefaultView().getCenter().x - gameOverText.getGlobalBounds().width / 2.f,
                window.getDefaultView().getCenter().y + 150.f);
            window.draw(gameOverText);
        }
        else {
            for (auto& s : bgTiles) window.draw(s);
            for (auto& a : drinks) if (!a.picked) window.draw(a.shape);
            for (auto* p : platforms) window.draw(p->shape);
            for (auto& h : hazards) window.draw(h.shape);
            if (wallActive)
                window.draw(giantWall);
            window.draw(doorSprite);
            window.draw(player.shape);
            window.draw(player.hpBar);
            for (auto& e : enemies) if (e.alive) window.draw(e.shape);
            for (auto& b : player.bullets) if (b.active) window.draw(b.shape);
            for (auto& e : enemies)
                for (auto& b : e.bullets) if (b.active) window.draw(b.shape);
        }
        window.display();
    }

    for (auto* p : platforms) delete p;
    return 0;
}