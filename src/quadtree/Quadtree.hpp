#pragma once

#include <vector>

#include "Node.hpp"

struct Quadtree {
  std::vector<Node> nodes;
  std::vector<size_t> parents;

  float limitX[2]{-100.f, 100.f};
  float limitY[2]{-100.f, 100.f};

  size_t subdivide(size_t node);

  void insert(const Body& body);

  void propagate();

  sf::Vector2f acceleration(sf::Vector2f pos, float theta, float epsilon) const;

  void clear(const Quad& rootQuad);
};

