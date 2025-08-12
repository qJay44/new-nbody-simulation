#pragma once

struct Body {
  sf::Vector2f pos;
  sf::Vector2f vel;
  float mass;

  Body(sf::Vector2f pos, float mass, sf::Vector2f vel = {0.f, 0.f})
    : pos(pos), vel(vel), mass(mass) {}

  void update(sf::Vector2f acc, float dt) {
    vel += acc * dt;
    pos += vel * dt;
  }
};

