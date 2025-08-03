#pragma once

#include "Quad.hpp"

struct Node {
  size_t children = 0;
  size_t next;
  Quad quad;
  sf::Vector2f massCenter;
  float mass = 0.f;

  Node(const Quad& quad, size_t next) : children(0), next(next), quad(quad), massCenter({}), mass(0.f) {}

  bool isBranch() const {
    return children != 0;
  }

  bool isEmpty() const {
    return mass == 0.f;
  }
};

