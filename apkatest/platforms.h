#pragma once
#include <SFML/Graphics.hpp>

class Platform {
public:
    sf::RectangleShape shape;
    Platform(float x, float y, float w, float h);
};
