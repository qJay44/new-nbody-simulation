#pragma once

struct Body {
  sf::Vector2f pos;
  float mass;

  void update(sf::Vector2f acc, float dt) {
    pos += acc * dt;
  }
};

