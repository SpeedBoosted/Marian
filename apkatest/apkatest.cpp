#include <SFML/Graphics.hpp>
#include <vector>
#include "klasy.h"

using namespace sf;

const float MOVE_SPEED = 4.0f;

int main() {
    RenderWindow window(VideoMode(800, 600), "Marian");
    window.setFramerateLimit(60);
    View view(FloatRect(0, 0, 800, 600)); //Kamera

    Player player(100, 500);

    std::vector<Platform> platforms;
    platforms.emplace_back(0, 550, 800, 50);
    platforms.emplace_back(200, 450, 120, 20);
    platforms.emplace_back(400, 350, 120, 20);
    platforms.emplace_back(600, 250, 120, 20);
    std::vector<Enemy> enemies;
    enemies.emplace_back(220, 410);

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
        }

        // Sterowanie
        if (Keyboard::isKeyPressed(Keyboard::Left))
            player.move(-MOVE_SPEED);
        if (Keyboard::isKeyPressed(Keyboard::Right))
            player.move(MOVE_SPEED);
        if (Keyboard::isKeyPressed(Keyboard::Space))
            player.jump();
        player.update(platforms);

        for (auto& enemy : enemies)
            enemy.update(platforms);

        // podąża kamera B)
        Vector2f playerCenter = player.shape.getPosition() + player.shape.getSize() / 2.f;
        view.setCenter(playerCenter);

        window.setView(view);

		// wizualizacjia
        window.clear(Color::Cyan);
		window.draw(player.shape);

        for (const auto& platform : platforms)
            window.draw(platform.shape);

        for (const auto& enemy : enemies)
            if (enemy.alive)
                window.draw(enemy.shape);
        window.display();
    }
    return 0;
}