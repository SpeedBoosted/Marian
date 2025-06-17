#include "platforms.h"

Platform::Platform(float x, float y, float w, float h) {
    shape.setPosition(x, y);
    shape.setSize({ w, h });
    shape.setFillColor(sf::Color(100, 250, 100));
}
