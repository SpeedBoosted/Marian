#include "klasy.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm> 

using namespace sf;
using namespace std;

static const float GRAVITY = 0.5f;
static const float JUMP_SPEED = -10.f;

// Inicjalizacja statycznych tekstur
Texture Platform::texture;
Texture Player::runTexture;
Texture Alcohol::beerTexture;
Texture Alcohol::wodkaTexture;
Texture Alcohol::kubusTexture;
Texture Alcohol::wineTexture;
Texture Enemy::pistolEnemyTexture;
Texture Enemy::shotgunEnemyTexture;
Texture Hazard::texture;

// --- Platform ---
Platform::Platform(float x, float y, float w, float h) {
    if (texture.getSize().x == 0) {
        if (!texture.loadFromFile("platform.png"))
            cerr << "platform.png missing\n";
        texture.setRepeated(true);
    }
    shape.setTexture(&texture);
    shape.setSize({ w,h });
    shape.setTextureRect({ 0,0,static_cast<int>(w),static_cast<int>(h) });
    shape.setPosition(x, y);
}

// --- MovingPlatform ---
MovingPlatform::MovingPlatform(float x, float y, float w, float h,
    const Vector2f& vel, float t)
    : Platform(x, y, w, h), origin(x, y), velocity(vel), travel(t)
{
}
void MovingPlatform::update() {
    shape.move(velocity);
    auto offs = shape.getPosition() - origin;
    if (abs(offs.x) > travel) velocity.x = -velocity.x;
    if (abs(offs.y) > travel) velocity.y = -velocity.y;
}

// --- Bullet ---
Bullet::Bullet(float x, float y, float vx, float vy)
    : velocity(vx, vy), active(true)
{
    shape.setPosition(x, y);
    shape.setSize({ 10.f,5.f });
    shape.setFillColor(Color::Yellow);
    damage = 10;
}
void Bullet::update() {
    if (active) shape.move(velocity);
}

// --- Player ---
Player::Player(float x, float y)
    : velocity(0.f, 0.f), onGround(false), hp(100)
{
    if (runTexture.getSize().x == 0) {
        if (!runTexture.loadFromFile("player_run.png"))
            cerr << "player_run.png missing\n";
    }
    int fw = runTexture.getSize().x / frameCount;
    int fh = runTexture.getSize().y;

    shape.setSize({ static_cast<float>(fw),static_cast<float>(fh) });
    shape.setTexture(&runTexture);
    shape.setTextureRect({ 0,0,fw,fh });
    shape.setOrigin(static_cast<float>(fw) / 2.f, static_cast<float>(fh) / 2.f);
    shape.setPosition(x + static_cast<float>(fw) / 2.f, y + static_cast<float>(fh) / 2.f);

    hpBar.setSize({ static_cast<float>(hp),10.f });
    hpBar.setFillColor(Color::Green);
    hpBar.setPosition(x, y - 15.f);
    shootClock.restart();
}
void Player::move(float dx) {
    velocity.x = dx; shape.move(dx, 0);
}
void Player::update(const vector<Platform*>& plats) {
    if (velocity.x != 0) {
        if (animClock.getElapsedTime().asSeconds() > timePerFrame) {
            animClock.restart();
            currentFrame = (currentFrame + 1) % frameCount;
            int fw = runTexture.getSize().x / frameCount;
            int fh = runTexture.getSize().y;
            shape.setTextureRect({ currentFrame * fw,0,fw,fh });
        }
        shape.setScale((velocity.x < 0 ? -1.f : 1.f), 1.f);
    }
    else {
        shape.setScale(1.f, 1.f);
        int fw = runTexture.getSize().x / frameCount;
        int fh = runTexture.getSize().y;
        shape.setTextureRect({ 0,0,fw,fh });
    }

    velocity.y += GRAVITY;
    shape.move(0, velocity.y);

    auto pos = shape.getPosition();
    hpBar.setPosition(pos.x - shape.getSize().x / 2.f,
        pos.y - shape.getSize().y / 2.f - 15.f);

    onGround = false;
    auto gb = shape.getGlobalBounds();
    for (auto* p : plats) {
        auto pb = p->shape.getGlobalBounds();
        if (gb.intersects(pb) && velocity.y >= 0) {
            // KLUCZOWA POPRAWKA: Ustawienie pozycji Y gracza idealnie na platformie.
            // Obliczenia uwzględniają 'origin' sprite'a.
            shape.setPosition(shape.getPosition().x, pb.top - shape.getGlobalBounds().height + shape.getOrigin().y);
            velocity.y = 0.f; onGround = true; break;
        }
    }
    for (auto& b : bullets) b.update();
    velocity.x = 0.f;
}
void Player::jump() {
    if (onGround) { velocity.y = JUMP_SPEED - jumpBoost; onGround = false; }
}
void Player::takeDamage(int amt) {
    hp = max(0, hp - amt);
    hpBar.setSize({ static_cast<float>(hp),10.f });
}
void Player::shoot(const Vector2f& tgt) {
    auto c = shape.getPosition();
    float dx = tgt.x - c.x, dy = tgt.y - c.y;
    float len = sqrt(dx * dx + dy * dy);
    if (shootClock.getElapsedTime().asSeconds() >= (0.15f * cooldownPenalty)) {
        if (len > 0) {
            Bullet b(c.x, c.y, dx / len * 8.f, dy / len * 8.f);
            b.damage = static_cast<int>(b.damage * damageBoost);
            bullets.push_back(b);
        }
        shootClock.restart();
    }
}

sf::FloatRect Player::getCollisionBounds() const {
    auto b = shape.getGlobalBounds();

    float reduceX = b.width * 0.20f;
    float reduceY = b.height * 0.10f;

    b.left += reduceX;
    b.width -= (reduceX * 2.f);

    b.top += reduceY;
    b.height -= (reduceY * 2.f);

    return b;
}

// --- Enemy ---
Enemy::Enemy(float x, float y, Type t)
    : speed(2.f), alive(true), hp(100), type(t),
    state(Patrol), detectionRange(500.f),
    retreatThreshold(30), chaseThreshold(60)
{
    float currentEnemyWidth;
    float currentEnemyHeight;

    if (type == PISTOL) {
        if (pistolEnemyTexture.getSize().x == 0) {
            if (!pistolEnemyTexture.loadFromFile("enemy1.png"))
                cerr << "enemy1.png missing\n";
        }
        shape.setTexture(&pistolEnemyTexture);
        currentEnemyWidth = static_cast<float>(pistolEnemyTexture.getSize().x) / pistolFrameCount;
        currentEnemyHeight = static_cast<float>(pistolEnemyTexture.getSize().y);
        shape.setSize({ currentEnemyWidth, currentEnemyHeight });
        shape.setTextureRect({ 0,0,static_cast<int>(currentEnemyWidth),static_cast<int>(currentEnemyHeight) });
        shape.setOrigin(currentEnemyWidth / 2.f, currentEnemyHeight / 2.f);
    }
    else if (type == SHOTGUN) {
        if (shotgunEnemyTexture.getSize().x == 0) {
            if (!shotgunEnemyTexture.loadFromFile("enemy2.png"))
                cerr << "enemy2.png missing\n";
        }
        shape.setTexture(&shotgunEnemyTexture);
        currentEnemyWidth = static_cast<float>(shotgunEnemyTexture.getSize().x) / shotgunFrameCount;
        currentEnemyHeight = static_cast<float>(shotgunEnemyTexture.getSize().y);
        shape.setSize({ currentEnemyWidth, currentEnemyHeight });
        shape.setTextureRect({ 0,0,static_cast<int>(currentEnemyWidth),static_cast<int>(currentEnemyHeight) });
        shape.setOrigin(currentEnemyWidth / 2.f, currentEnemyHeight / 2.f);
    }
    else {
        currentEnemyWidth = 40.f;
        currentEnemyHeight = 40.f;
        shape.setSize({ currentEnemyWidth, currentEnemyHeight });
        shape.setFillColor(Color(139, 69, 19));
        shape.setOrigin(currentEnemyWidth / 2.f, currentEnemyHeight / 2.f);
    }

    shape.setPosition(x, y - currentEnemyHeight / 2.f);

    shootCooldown = (type == PISTOL ? 0.8f : (type == SHOTGUN ? 1.5f : 0.5f));
    shootClock.restart();
    pistolAnimClock.restart();
    shotgunAnimClock.restart();
}

void Enemy::update(const std::vector<Platform*>& plats, const Player& pl, sf::Sound& snd) {
    if (type == BOSS) {
        bossCenter = sf::Vector2f(1000.f, 500.f);
        bossTime += 0.05f;
        float t = bossTime * 0.5f;
        float a = bossRadius;
        // Ruch bossa po lemniskacie Bernoulliego (ósemka)
        float x = a * std::sin(t) / (1.f + std::pow(std::cos(t), 2.f));
        float y = a * std::sin(t) * std::cos(t) / (1.f + std::pow(std::cos(t), 2.f));
        shape.setPosition(bossCenter.x + x, bossCenter.y + y);

        if (shootClock.getElapsedTime().asSeconds() >= shootCooldown) {
            shootBossAttack(pl);
            snd.play();
            shootClock.restart();
        }
        for (auto& b : bullets) b.update();
        return;
    }
    if (!alive) return;
    bool onPlat = false; const Platform* base = nullptr;
    auto eB = shape.getGlobalBounds();
    float currentEnemyHeight = shape.getSize().y;

    for (auto* p : plats) {
        auto pB = p->shape.getGlobalBounds();
        if (abs((eB.top + eB.height) - pB.top) < 5.f &&
            eB.left < pB.left + pB.width &&
            eB.left + eB.width > pB.left)
        {
            onPlat = true;
            base = p;
            shape.setPosition(shape.getPosition().x, pB.top - currentEnemyHeight / 2.f);
            break;
        }
    }
    if (!onPlat) shape.move(0.f, GRAVITY);

    auto eC = shape.getPosition();
    auto pC = pl.shape.getPosition();
    float dx = pC.x - eC.x, dy = pC.y - eC.y;
    float dist = sqrt(dx * dx + dy * dy);
    bool see = canSeePlayer(pl, plats) && dist < detectionRange;

    // Animacja wroga
    if (type == PISTOL || type == SHOTGUN) {
        sf::Texture* currentTexture = nullptr;
        int* currentFramePtr = nullptr;
        int frameCountVal = 0;
        float timePerFrameVal = 0;
        sf::Clock* animClockPtr = nullptr;

        if (type == PISTOL) {
            currentTexture = &pistolEnemyTexture;
            currentFramePtr = &currentPistolFrame;
            frameCountVal = pistolFrameCount;
            timePerFrameVal = pistolTimePerFrame;
            animClockPtr = &pistolAnimClock;
        }
        else if (type == SHOTGUN) {
            currentTexture = &shotgunEnemyTexture;
            currentFramePtr = &currentShotgunFrame;
            frameCountVal = shotgunFrameCount;
            timePerFrameVal = shotgunTimePerFrame;
            animClockPtr = &shotgunAnimClock;
        }

        if (currentTexture && animClockPtr) {
            if (abs(speed) > 0.01f || state == Chase) {
                if (animClockPtr->getElapsedTime().asSeconds() > timePerFrameVal) {
                    animClockPtr->restart();
                    *currentFramePtr = (*currentFramePtr + 1) % frameCountVal;
                    int fw = currentTexture->getSize().x / frameCountVal;
                    int fh = currentTexture->getSize().y;
                    shape.setTextureRect({ (*currentFramePtr) * fw, 0, fw, fh });
                }
            }
            else {
                int fw = currentTexture->getSize().x / frameCountVal;
                int fh = currentTexture->getSize().y;
                shape.setTextureRect({ 0, 0, fw, fh });
                *currentFramePtr = 0;
            }

            // Obracanie sprite'a
            if (state == Patrol) {
                if (speed < 0) {
                    shape.setScale(-1.f, 1.f);
                }
                else if (speed > 0) {
                    shape.setScale(1.f, 1.f);
                }
            }
            else if (state == Chase || state == Attack) {
                if (dx < 0) {
                    shape.setScale(-1.f, 1.f);
                }
                else if (dx > 0) {
                    shape.setScale(1.f, 1.f);
                }
            }
            else if (state == Retreat) {
                if (dx < 0) {
                    shape.setScale(1.f, 1.f);
                }
                else if (dx > 0) {
                    shape.setScale(-1.f, 1.f);
                }
            }
        }
    }

    // Maszyna stanów wroga
    switch (state) {
    case Patrol:
        if (onPlat) {
            auto pB = base->shape.getGlobalBounds();
            float currentShapeLeft = shape.getPosition().x - shape.getOrigin().x;
            float currentShapeRight = shape.getPosition().x - shape.getOrigin().x + shape.getSize().x;

            if (speed > 0) {
                if (currentShapeRight + speed > pB.left + pB.width - 2.f) {
                    speed = -speed;
                }
                else {
                    shape.move(speed, 0);
                }
            }
            else {
                if (currentShapeLeft + speed < pB.left + 2.f) {
                    speed = -speed;
                }
                else {
                    shape.move(speed, 0);
                }
            }
        }
        if (see) state = Chase;
        break;
    case Chase:
        if (!see) { state = Patrol; break; }
        shape.move((dx > 0 ? 1.f : -1.f) * speed, 0);
        state = Attack;
        break;
    case Attack:
        if (!see) { state = Patrol; break; }
        if (hp < retreatThreshold) { state = Retreat; break; }
        if (shootClock.getElapsedTime().asSeconds() >= shootCooldown) {
            if (type == PISTOL) shootPistol(pl);
            else        shootShotgun(pl);
            snd.play();
            shootClock.restart();
        }
        break;
    case Retreat:
        if (hp > chaseThreshold) { state = Patrol; break; }
        shape.move((dx > 0 ? -1.f : 1.f) * speed, 0);
        break;
    }
    for (auto& b : bullets) b.update();
}
void Enemy::shootPistol(const Player& pl) {
    auto e = shape.getPosition();
    auto p = pl.shape.getPosition();
    float dx = p.x - e.x, dy = p.y - e.y;
    float len = sqrt(dx * dx + dy * dy);
    if (len > 0) bullets.emplace_back(e.x, e.y, dx / len * 5.f, dy / len * 5.f);
}
void Enemy::shootShotgun(const Player& pl) {
    auto e = shape.getPosition();
    auto p = pl.shape.getPosition();
    float baseAng = atan2(p.y - e.y, p.x - e.x);
    const int pellets = 5;
    const float spread = 15.f * 3.14159f / 180.f, spd = 6.f;
    for (int i = 0;i < pellets;i++) {
        float ang = baseAng + (static_cast<float>(i) - static_cast<float>(pellets) / 2.f) * spread;
        bullets.emplace_back(e.x, e.y, cos(ang) * spd, sin(ang) * spd);
    }
}
void Enemy::shootBossAttack(const Player& pl) {
    auto e = shape.getPosition();
    auto p = pl.shape.getPosition();
    float dx = p.x - e.x, dy = p.y - e.y;
    float len = sqrt(dx * dx + dy * dy);
    if (len > 0) {
        Bullet b(e.x, e.y, dx / len * 8.f, dy / len * 8.f);
        b.shape.setSize({ 30.f, 15.f });
        b.shape.setFillColor(sf::Color::Red);
        b.damage = 30;
        bullets.push_back(b);
    }
}
void Enemy::takeDamage(int amt) {
    hp = max(0, hp - amt);
    if (hp == 0) alive = false;
}
bool Enemy::canSeePlayer(const Player& pl,
    const vector<Platform*>& plats)
{
    auto E = shape.getPosition();
    auto P = pl.shape.getPosition();
    auto dir = P - E; float len = sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len == 0.f) return true;
    dir /= len;
    for (float d = 0.f;d < len;d += 5.f) {
        auto sample = E + dir * d;
        for (auto* p : plats) {
            if (p->shape.getGlobalBounds().contains(sample))
                return false;
        }
    }
    return true;
}

// --- Hazard ---
Hazard::Hazard(float x, float y, float w, float h) {
    if (texture.getSize().x == 0) {
        if (!texture.loadFromFile("lava.png")) {
            cerr << "ERROR: lava.png missing or could not be loaded! Please ensure it's in the same directory as the executable.\n";
        }
        else {
            cerr << "SUCCESS: lava.png loaded. Size: " << texture.getSize().x << "x" << texture.getSize().y << "\n";
            texture.setRepeated(true);
            texture.setSmooth(false);
        }
    }
    shape.setTexture(&texture);
    shape.setSize({ w,h });
    shape.setTextureRect(sf::IntRect(0, 0, static_cast<int>(w), static_cast<int>(h)));
    shape.setPosition(x, y);
    damageClock.restart();
}
void Hazard::update(Player& p) {
    // Zadawanie obrażeń co sekundę
    if (shape.getGlobalBounds().intersects(p.getCollisionBounds())) {
        if (damageClock.getElapsedTime().asSeconds() >= 1.f) {
            p.takeDamage(10);
            damageClock.restart();
        }
    }
    else {
        // Reset zegara, gdy gracz opuści pułapkę
        damageClock.restart();
    }
}

// Konstruktor Alcohol
Alcohol::Alcohol(AlcoholType t, float x, float y) : type(t) {
    shape.setSize({ 30.f, 30.f });
    shape.setPosition(x, y);

    // Ładowanie i przypisywanie tekstur
    if (beerTexture.getSize().x == 0) { if (!beerTexture.loadFromFile("beer.png")) cerr << "beer.png missing\n"; }
    if (wodkaTexture.getSize().x == 0) { if (!wodkaTexture.loadFromFile("wodka.png")) cerr << "wodka.png missing\n"; }
    if (kubusTexture.getSize().x == 0) { if (!kubusTexture.loadFromFile("kubus.png")) cerr << "kubus.png missing\n"; }
    if (wineTexture.getSize().x == 0) { if (!wineTexture.loadFromFile("wine.png")) cerr << "wine.png missing\n"; }
    switch (t) {
    case AlcoholType::Piwo: shape.setTexture(&beerTexture); break;
    case AlcoholType::Wodka: shape.setTexture(&wodkaTexture); break;
    case AlcoholType::Kubus: shape.setTexture(&kubusTexture); break;
    case AlcoholType::Wino: shape.setTexture(&wineTexture); break;
    }
}

void Player::pickUpAlcohol(std::vector<Alcohol>& drinks) {
    for (auto it = drinks.begin(); it != drinks.end(); ) {
        // Sprawdzenie kolizji z "potkiem" i podniesienie go
        if (!it->picked && getCollisionBounds().intersects(it->shape.getGlobalBounds())) {
            alcoholInventory[it->type]++;
            it->picked = true;
            std::cout << "Picked up alcohol type: " << static_cast<int>(it->type) << ". Count: " << alcoholInventory[it->type] << "\n";
            it = drinks.erase(it);

            std::ofstream out("potki.txt", std::ios::trunc);
            if (!out.is_open()) { std::cerr << "Could not open potki.txt for writing.\n"; }
            else {
                for (const auto& pair : alcoholInventory) { out << static_cast<int>(pair.first) << " " << pair.second << "\n"; }
                out.close();
            }
        }
        else {
            ++it;
        }
    }
}

void Player::useAlcohol() {
    if (alcoholInventory[selectedAlcohol] > 0) {
        alcoholInventory[selectedAlcohol]--;

        // Zastosowanie efektów alkoholu
        switch (selectedAlcohol) {
        case AlcoholType::Piwo: jumpBoost += 0.5f; std::cout << "Uzyto piwa. Zwiekszona sila skoku!\n"; break;
        case AlcoholType::Wodka: cooldownPenalty *= 2.f; std::cout << "Uzyto wodki. Zwiekszony cooldown strzelania!\n"; break;
        case AlcoholType::Kubus: speedBoost += 0.1f; std::cout << "Uzyto kubusia. Zwiekszona predkosc ruchu!\n"; break;
        case AlcoholType::Wino: damageBoost += 0.2f; std::cout << "Uzyto wina. Zwiekszone obrazenia strzalow!\n"; break;
        }

        std::ofstream out("potki.txt", std::ios::trunc);
        if (!out.is_open()) { std::cerr << "Could not open potki.txt for writing.\n"; return; }
        for (const auto& pair : alcoholInventory) { out << static_cast<int>(pair.first) << " " << pair.second << "\n"; }
        out.close();
    }
    else { std::cout << "Brak " << static_cast<int>(selectedAlcohol) << " w ekwipunku!\n"; }
}

void Player::resetAlcoholEffects() {
    jumpBoost = 0.f; speedBoost = 1.f; damageBoost = 1.f; cooldownPenalty = 1.f;
    alcoholInventory.clear();
    hp = 100;
    hpBar.setSize({ static_cast<float>(hp), 10.f });
    std::ofstream out("potki.txt", std::ios::trunc); out.close();
}

// --- Menu ---
Menu::Menu() :inMenu(true), selectedLevel(0) {
    if (!font.loadFromFile("flame.otf")) cerr << "flame.otf missing\n";
    t1.setFont(font); t1.setString("1: Level 1"); t1.setPosition(300.f, 100.f);
    t2.setFont(font); t2.setString("2: Level 2"); t2.setPosition(300.f, 150.f);
    t3.setFont(font); t3.setString("3: Level 3"); t3.setPosition(300.f, 200.f);
    t4.setFont(font); t4.setString("4: Level 4"); t4.setPosition(300.f, 250.f);
    t5.setFont(font); t5.setString("5: Level 5"); t5.setPosition(300.f, 300.f);
    if (!bgTexture.loadFromFile("tlo_menu.png")) cerr << "tlo_menu.png missing\n";
    bgSprite.setTexture(bgTexture);
}
void Menu::handleInput() {
    if (Keyboard::isKeyPressed(Keyboard::Num1)) { selectedLevel = 1; inMenu = false; }
    if (Keyboard::isKeyPressed(Keyboard::Num2)) { selectedLevel = 2; inMenu = false; }
    if (Keyboard::isKeyPressed(Keyboard::Num3)) { selectedLevel = 3; inMenu = false; }
    if (Keyboard::isKeyPressed(Keyboard::Num4)) { selectedLevel = 4; inMenu = false; }
    if (Keyboard::isKeyPressed(Keyboard::Num5)) { selectedLevel = 5; inMenu = false; }
}
void Menu::draw(RenderWindow& w) {
    auto ws = w.getSize();
    float sx = static_cast<float>(ws.x) / bgTexture.getSize().x;
    float sy = static_cast<float>(ws.y) / bgTexture.getSize().y;
    bgSprite.setScale(sx, sy);
    w.draw(bgSprite);
    w.draw(t1); w.draw(t2); w.draw(t3); w.draw(t4); w.draw(t5);
}