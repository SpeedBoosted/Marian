#include "klasy.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <iostream>
#include <fstream>
using namespace sf;
using namespace std;

static const float GRAVITY = 0.5f;
static const float JUMP_SPEED = -10.f;

// Inicjalizacja statycznych tekstur dla Platform i Player
Texture Platform::texture;
Texture Player::runTexture;

// Inicjalizacja statycznych tekstur dla Alcohol
Texture Alcohol::beerTexture;
Texture Alcohol::wodkaTexture;
Texture Alcohol::kubusTexture;
Texture Alcohol::wineTexture;

// Inicjalizacja statycznej tekstury dla wroga PISTOL
Texture Enemy::pistolEnemyTexture;
// Inicjalizacja statycznej tekstury dla wroga SHOTGUN
Texture Enemy::shotgunEnemyTexture;

// Inicjalizacja statycznej tekstury dla Hazard
Texture Hazard::texture; // Dodano inicjalizację statycznej tekstury

// --- Platform ---
Platform::Platform(float x, float y, float w, float h) {
    if (texture.getSize().x == 0) {
        if (!texture.loadFromFile("platform.png"))
            cerr << "platform.png missing\n";
        texture.setRepeated(true);
    }
    shape.setTexture(&texture);
    shape.setSize({ w,h });
    shape.setTextureRect({ 0,0,int(w),int(h) });
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
    shape.setSize({ 10,5 });
    shape.setFillColor(Color::Yellow);
    damage = 10;
}
void Bullet::update() {
    if (active) shape.move(velocity);
}

// --- Player ---
Player::Player(float x, float y)
    : velocity(0, 0), onGround(false), hp(100)
{
    if (runTexture.getSize().x == 0) {
        if (!runTexture.loadFromFile("player_run.png"))
            cerr << "player_run.png missing\n";
    }
    int fw = runTexture.getSize().x / frameCount;
    int fh = runTexture.getSize().y;

    shape.setSize({ float(fw),float(fh) }); 
    shape.setTexture(&runTexture);
    shape.setTextureRect({ 0,0,fw,fh });
    shape.setOrigin(fw / 2.f, fh / 2.f);
    shape.setPosition(x + fw / 2.f, y + fh / 2.f);

    hpBar.setSize({ float(hp),10 });
    hpBar.setFillColor(Color::Green);
    hpBar.setPosition(x, y - 15.f);
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
        shape.setScale(1, 1);
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
            shape.setPosition(gb.left + gb.width / 2.f,
                pb.top - gb.height / 2.f);
            velocity.y = 0; onGround = true; break;
        }
    }
    for (auto& b : bullets) b.update();
    velocity.x = 0;
}
void Player::jump() {
    if (onGround) { velocity.y = JUMP_SPEED; onGround = false; }
}
void Player::takeDamage(int amt) {
    hp = max(0, hp - amt);
    hpBar.setSize({ float(hp),10 });
}
void Player::shoot(const Vector2f& tgt) {
    auto c = shape.getPosition();
    float dx = tgt.x - c.x, dy = tgt.y - c.y;
    float len = sqrt(dx * dx + dy * dy);
    if (len > 0) bullets.emplace_back(c.x, c.y, dx / len * 8.f, dy / len * 8.f);
}

sf::FloatRect Player::getCollisionBounds() const {
    auto b = shape.getGlobalBounds(); // Pobierz globalne granice sprite'a

    float reduceX = b.width * 0.40f; // Daje 20% szerokości
    float reduceY = b.height * 0.25f; // Daje 50% wysokości

    // Zastosuj zmniejszenia
    b.left += reduceX; // Przesuń lewą krawędź w prawo
    b.width -= (reduceX * 2); // Zmniejsz szerokość o podwójną wartość reduceX

    b.top += reduceY; // Przesuń górną krawędź w dół
    b.height -= (reduceY * 2); // Zmniejsz wysokość o podwójną wartość reduceY

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
        currentEnemyWidth = (float)pistolEnemyTexture.getSize().x / pistolFrameCount;
        currentEnemyHeight = (float)pistolEnemyTexture.getSize().y;
        shape.setSize({ currentEnemyWidth, currentEnemyHeight });
        shape.setTextureRect({ 0,0,(int)currentEnemyWidth,(int)currentEnemyHeight });
        shape.setOrigin(currentEnemyWidth / 2.f, currentEnemyHeight / 2.f);
    }
    else if (type == SHOTGUN) { // Ładowanie tekstury dla SHOTGUN
        if (shotgunEnemyTexture.getSize().x == 0) {
            if (!shotgunEnemyTexture.loadFromFile("enemy2.png")) // Upewnij się, że nazwa pliku jest poprawna
                cerr << "enemy2.png missing\n";
        }
        shape.setTexture(&shotgunEnemyTexture);
        currentEnemyWidth = (float)shotgunEnemyTexture.getSize().x / shotgunFrameCount;
        currentEnemyHeight = (float)shotgunEnemyTexture.getSize().y;
        shape.setSize({ currentEnemyWidth, currentEnemyHeight });
        shape.setTextureRect({ 0,0,(int)currentEnemyWidth,(int)currentEnemyHeight });
        shape.setOrigin(currentEnemyWidth / 2.f, currentEnemyHeight / 2.f);
    }
    else { // Dla BOSS (domyślne wartości 40x40)
        currentEnemyWidth = 40.f;
        currentEnemyHeight = 40.f;
        shape.setSize({ currentEnemyWidth, currentEnemyHeight });
        shape.setFillColor(Color(139, 69, 19));
        shape.setOrigin(currentEnemyWidth / 2.f, currentEnemyHeight / 2.f);
    }

    shape.setPosition(x, y - currentEnemyHeight / 2.f);

    shootCooldown = (type == PISTOL ? 0.8f : (type == SHOTGUN ? 1.5f : 0.5f)); // Dostosuj cooldown dla Shotguna
    shootClock.restart();
    pistolAnimClock.restart();
    shotgunAnimClock.restart(); // Uruchomienie zegara animacji dla SHOTGUN
}

void Enemy::update(const std::vector<Platform*>& plats, const Player& pl, sf::Sound& snd) {
    if (type == BOSS) {
        bossCenter = sf::Vector2f(1000, 500);
        bossRadius = 300;
        bossTime += 0.1f;
        float t = bossTime * 0.5f;
        float a = bossRadius;
        float x = a * std::sin(t) / (1 + std::pow(std::cos(t), 2));
        float y = a * std::sin(t) * std::cos(t) / (1 + std::pow(std::cos(t), 2));
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
    if (!onPlat) shape.move(0, GRAVITY);

    auto eC = shape.getPosition();
    auto pC = pl.shape.getPosition();
    float dx = pC.x - eC.x, dy = pC.y - eC.y;
    float dist = sqrt(dx * dx + dy * dy);
    bool see = canSeePlayer(pl, plats) && dist < detectionRange;

    // Animacja dla wroga PISTOL i SHOTGUN
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

            // Logika obracania sprite'a
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
                if (dx < 0) { // Gracz na lewo, wróg ucieka w prawo
                    shape.setScale(1.f, 1.f);
                }
                else if (dx > 0) { // Gracz na prawo, wróg ucieka w lewo
                    shape.setScale(-1.f, 1.f);
                }
            }
        }
    }


    switch (state) {
    case Patrol:
        if (onPlat) {
            auto pB = base->shape.getGlobalBounds();
            float currentShapeLeft = shape.getPosition().x - shape.getOrigin().x;
            float currentShapeRight = shape.getPosition().x - shape.getOrigin().x + shape.getSize().x;

            if (speed > 0) {
                if (currentShapeRight + speed > pB.left + pB.width - 2) {
                    speed = -speed;
                }
                else {
                    shape.move(speed, 0);
                }
            }
            else {
                if (currentShapeLeft + speed < pB.left + 2) {
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
    const float spread = 15 * 3.14159f / 180.f, spd = 6.f;
    for (int i = 0;i < pellets;i++) {
        float ang = baseAng + (i - pellets / 2) * spread;
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
        b.shape.setSize({ 30, 15 });
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
    if (len == 0) return true;
    dir /= len;
    for (float d = 0;d < len;d += 5.f) {
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
    if (texture.getSize().x == 0) { // Sprawdź, czy tekstura jest już załadowana
        if (!texture.loadFromFile("lava.png")) {
            cerr << "ERROR: lava.png missing or could not be loaded! Please ensure it's in the same directory as the executable.\n";
        }
        else {
            cerr << "SUCCESS: lava.png loaded. Size: " << texture.getSize().x << "x" << texture.getSize().y << "\n";
            texture.setRepeated(true);  // Ustawienie powtarzania
            texture.setSmooth(false);   // Wyłączenie wygładzania dla pikselowej tekstury
        }
    }
    shape.setTexture(&texture); // Ustaw teksturę
    shape.setSize({ w,h });
    shape.setTextureRect(sf::IntRect(0, 0, (int)w, (int)h));
    shape.setPosition(x, y);
    damageClock.restart();
}
void Hazard::update(Player& p) {
    // Używamy getCollisionBounds() gracza do detekcji kolizji
    if (shape.getGlobalBounds().intersects(p.getCollisionBounds())) {
        if (damageClock.getElapsedTime().asSeconds() >= 1.f) {
            p.takeDamage(10);
            damageClock.restart();
        }
    }
    else {
        damageClock.restart();
    }
}

// Zmieniony konstruktor Alcohol
Alcohol::Alcohol(AlcoholType t, float x, float y) : type(t) {
    shape.setSize({ 30.f, 30.f });
    shape.setPosition(x, y);

    // Ładowanie tekstur, jeśli jeszcze nie są załadowane
    if (beerTexture.getSize().x == 0) {
        if (!beerTexture.loadFromFile("beer.png"))
            cerr << "beer.png missing\n";
    }
    if (wodkaTexture.getSize().x == 0) {
        if (!wodkaTexture.loadFromFile("wodka.png"))
            cerr << "wodka.png missing\n";
    }
    if (kubusTexture.getSize().x == 0) {
        if (!kubusTexture.loadFromFile("kubus.png"))
            cerr << "kubus.png missing\n";
    }
    if (wineTexture.getSize().x == 0) {
        if (!wineTexture.loadFromFile("wine.png"))
            cerr << "wine.png missing\n";
    }

    // Przypisywanie tekstury na podstawie typu alkoholu
    switch (t) {
    case AlcoholType::Piwo: shape.setTexture(&beerTexture); break;
    case AlcoholType::Wodka: shape.setTexture(&wodkaTexture); break;
    case AlcoholType::Kubus: shape.setTexture(&kubusTexture); break;
    case AlcoholType::Wino: shape.setTexture(&wineTexture); break;
    }
}

void Player::pickUpAlcohol(std::vector<Alcohol>& drinks) {
    for (auto& a : drinks) {
        // Używamy getCollisionBounds() gracza do detekcji kolizji
        if (!a.picked && getCollisionBounds().intersects(a.shape.getGlobalBounds())) {
            alcoholInventory[a.type]++;
            a.picked = true;
            std::cout << "Picked up alcohol type: " << static_cast<int>(a.type) << "\n";

            std::ofstream out("potki.txt", std::ios::trunc);
            if (!out.is_open()) {
                std::cerr << "Could not open potki.txt for writing.\n";
                continue;
            }
            for (const auto& pair : alcoholInventory) {
                out << static_cast<int>(pair.first) << " " << pair.second << "\n";
            }
        }
    }
}


void Player::useAlcohol() {
    if (alcoholInventory[selectedAlcohol] > 0) {
        alcoholInventory[selectedAlcohol]--;

        switch (selectedAlcohol) {
        case AlcoholType::Piwo: jumpBoost += 0.5f; break;
        case AlcoholType::Wodka: cooldownPenalty *= 2.f; break;
        case AlcoholType::Kubus: speedBoost += 0.1f; break;
        case AlcoholType::Wino: damageBoost += 0.2f; break;
        }

        std::ofstream out("potki.txt", std::ios::trunc);
        if (!out.is_open()) {
            std::cerr << "Could not open potki.txt for writing.\n";
            return;
        }
        for (const auto& pair : alcoholInventory) {
            out << static_cast<int>(pair.first) << " " << pair.second << "\n";
        }
    }
}

void Player::resetAlcoholEffects() {
    jumpBoost = 0.f;
    speedBoost = 1.f;
    damageBoost = 1.f;
    cooldownPenalty = 1.f;
    alcoholInventory.clear();
    hp = 100;
    hpBar.setSize({ float(hp), 10 });
    std::ofstream out("potki.txt", std::ios::trunc);
    out.close();
}


// --- Menu ---
Menu::Menu() :inMenu(true), selectedLevel(0) {
    if (!font.loadFromFile("flame.otf")) cerr << "flame.otf missing\n";
    t1.setFont(font); t1.setString("1: Level 1"); t1.setPosition(300, 100);
    t2.setFont(font); t2.setString("2: Level 2"); t2.setPosition(300, 150);
    t3.setFont(font); t3.setString("3: Level 3"); t3.setPosition(300, 200);
    t4.setFont(font); t4.setString("4: Level 4"); t4.setPosition(300, 250);
    t5.setFont(font); t5.setString("5: Level 5"); t5.setPosition(300, 300);
    if (!bgTexture.loadFromFile("tlo_menu.png"))
        cerr << "tlo_menu.png missing\n";
    bgSprite.setTexture(bgTexture);
}
void Menu::handleInput() {
    if (Keyboard::isKeyPressed(Keyboard::Num1))
    {
        selectedLevel = 1; inMenu = false;
    }
    if (Keyboard::isKeyPressed(Keyboard::Num2))
    {
        selectedLevel = 2; inMenu = false;
    }
    if (Keyboard::isKeyPressed(Keyboard::Num3))
    {
        selectedLevel = 3; inMenu = false;
    }
    if (Keyboard::isKeyPressed(Keyboard::Num4))
    {
        selectedLevel = 4; inMenu = false;
    }
    if (Keyboard::isKeyPressed(Keyboard::Num5))
    {
        selectedLevel = 5; inMenu = false;
    }
}
void Menu::draw(RenderWindow& w) {
    auto ws = w.getSize();
    float sx = float(ws.x) / bgTexture.getSize().x;
    float sy = float(ws.y) / bgTexture.getSize().y;
    bgSprite.setScale(sx, sy);
    w.draw(bgSprite);
    w.draw(t1); w.draw(t2); w.draw(t3); w.draw(t4); w.draw(t5);
}