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

    // GameOver/YouWin
    Font font; font.loadFromFile("arial.ttf");
    Text gameOver("Rip BOZO!\nPress any key...", font, 40);
    gameOver.setFillColor(Color::Red);
    gameOver.setPosition(200, 250);

    // Napisy
    sf::Music endingMusic;
    if (!endingMusic.openFromFile("ending.wav"))
        std::cerr << "ending.wav missing\n";
    endingMusic.setLoop(false);
    Texture logoTex;
    if (!logoTex.loadFromFile("logo.png"))
        std::cerr << "logo.png missing\n";
    Sprite logoSprite(logoTex);
    logoSprite.setOrigin(logoTex.getSize().x / 2.f, logoTex.getSize().y / 2.f);
    logoSprite.setPosition(400, 120);
    bool inCredits = false;
    float creditsY = 600.f;
    const float creditsSpeed = 50.f; // px/s
    std::vector<std::string> creditsLines = {
        "Autorzy:","Jan Dymek","Jan Dymek",
        "",
        "W rolach głownych","Marian - Martin Yan","Przeciwnik 1 - Jason Mamoa","Przeciwnik 2 - Jan Dymek",
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
    RectangleShape giantWall;
    bool wallActive = false;
    float wallSpeed = 0.f;
	// muzowanie
    auto playMusic = [&](sf::Music& music) {
        menuMusic.stop();
        level1Music.stop();
        level2Music.stop();
        level3Music.stop();
        music.play();
        };
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
                playMusic(level1Music);
                break;
                wallActive = false;
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
                playMusic(level2Music);
                wallActive = false;
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
                playMusic(level1Music);
                wallActive = false;
                break;
            case 4:
                levelWidth = 2000.f;
                player = Player(50, 500);
                platforms.push_back(new Platform(0, 550, levelWidth, 50));
                platforms.push_back(new Platform(400, 400, 200, 20));
                platforms.push_back(new Platform(800, 300, 200, 20));
                platforms.push_back(new Platform(1400, 400, 200, 20));
                // Boss w środku poziomu
                enemies.emplace_back(1000, 250, Enemy::BOSS);
                // Możesz dodać inne przeszkody/hazardy
                playMusic(level2Music);
                wallActive = false;
                break;

            case 5:
                levelWidth = 2500.f;
                player = Player(50, 500);
                giantWall.setSize(Vector2f(60, 600));
                giantWall.setFillColor(Color(120, 120, 120));
                giantWall.setPosition(0, 0);
                wallActive = true;
                wallSpeed = 3.f;
                platforms.push_back(new Platform(0, 550, levelWidth, 50));
                platforms.push_back(new Platform(500, 450, 200, 20));
                platforms.push_back(new Platform(1000, 350, 200, 20));
                platforms.push_back(new Platform(1600, 250, 200, 20));
                hazards.emplace_back(800, 530, 150, 20);
                hazards.emplace_back(1800, 530, 200, 20);
                playMusic(level3Music);
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
        float deltaTime = frameClock.restart().asSeconds();
        // napisy
        if(inCredits) {
            if (endingMusic.getStatus() != sf::Music::Playing)
                playMusic(endingMusic);

            window.setView(window.getDefaultView());
            window.clear(Color::Black);
            std::string allLines;
            for (const auto& line : creditsLines)
                allLines += line + "\n";
            creditsText.setString(allLines);
            // Wyśrodkuj logo
            float logoY = creditsY;
            logoSprite.setPosition(400, logoY + logoTex.getSize().y / 2.f);
            // tekst
            float textStartY = logoY + logoTex.getSize().y + 20.f;
            creditsText.setPosition(
                400 - creditsText.getLocalBounds().width / 2.f,
                textStartY
            );
            creditsY -= creditsSpeed * deltaTime;

            window.draw(logoSprite);
            window.draw(creditsText);
            window.display();

            // wróć do menu
            if (textStartY + creditsText.getLocalBounds().height < 0) {
                inCredits = false;
                menu.inMenu = true;
                playMusic(menuMusic);
            }
            continue;
        }
        Event ev;
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
            if (menuMusic.getStatus() != sf::Music::Playing)
                playMusic(menuMusic);
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
                if (!e.alive) continue;
                Sound& snd = (e.type == Enemy::PISTOL ? pistolSound : (e.type == Enemy::SHOTGUN ? shotgunSound : bossSound));
                e.update(platforms, player, snd);
                if (wallActive) {
                    Vector2f pos = giantWall.getPosition();
                    pos.x += wallSpeed * deltaTime;
                    if (pos.x + giantWall.getSize().x > levelWidth)
                        pos.x = levelWidth - giantWall.getSize().x;
                    giantWall.setPosition(pos);

                    if (giantWall.getGlobalBounds().intersects(player.shape.getGlobalBounds())) {
                        player.hp = 0;
                        isGameOver = true;
                    }
                }

                for (auto& b : player.bullets)
                    if (b.active && b.shape.getGlobalBounds()
                        .intersects(e.shape.getGlobalBounds())) {
                        if (e.type == Enemy::BOSS) {
                            e.hits++;
                            if (e.hits >= 5) {
                                e.alive = false;
                            }
                        }
                        else {
                            e.takeDamage(40);
                        }
                        b.active = false;
                    }
                for (auto& b : e.bullets)
                    if (b.active && b.shape.getGlobalBounds()
                        .intersects(player.getCollisionBounds())) {
                        player.takeDamage(10); b.active = false;
                    }
            }

            if (player.hp <= 0) isGameOver = true;
            if (player.shape.getGlobalBounds().intersects(goal.getGlobalBounds())) {
                // Sprawdź czy to poziom 4 i boss żyje
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
                    // Nie pozwól ukończyć poziomu
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

        // kamera
        Vector2f cam = player.shape.getPosition() + player.shape.getSize() / 2.f;
        cam.x = std::max(400.f, std::min(cam.x, levelWidth - 400.f));
        view.setCenter(cam);
        window.setView(view);

        // render
        window.clear(Color::Cyan);
        if (isGameOver) {
            window.setView(window.getDefaultView());
            window.draw(gameOver);
        }
        else {
            for (auto& s : bgTiles) window.draw(s);
            for (auto* p : platforms) window.draw(p->shape);
            for (auto& h : hazards)   window.draw(h.shape);
            if (wallActive)
                window.draw(giantWall);
            window.draw(goal);
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
