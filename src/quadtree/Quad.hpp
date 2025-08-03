#pragma once

#include <cfloat>
#include <array>

#include "Body.hpp"

struct Quad {
  sf::Vector2f center;
  float size;

  static Quad calcRootQuad(const std::vector<Body>& bodies) {
    float minX = FLT_MAX;
    float minY = FLT_MAX;
    float maxX = -FLT_MAX;
    float maxY = -FLT_MAX;

    for (const Body& body : bodies) {
      minX = std::min(minX, body.pos.x);
      minY = std::min(minY, body.pos.y);
      maxX = std::max(maxX, body.pos.x);
      maxY = std::max(maxY, body.pos.y);
    }

    sf::Vector2f center = sf::Vector2f{minX + maxX, minY + maxY} * 0.5f;
    float size = std::max(maxX - minX, maxY - minY);

    return {center, size};
  }

  Quad() {}

  Quad(sf::Vector2f center, float size) : center(center), size(size) {}

  std::array<Quad, 4> intoQuadrants() const {
    std::array<Quad, 4> quads;
    float sizeHalf = size * 0.5f;

    quads[1] = {{center.x - sizeHalf, center.y - sizeHalf}, sizeHalf};
    quads[0] = {{center.x + sizeHalf, center.y - sizeHalf}, sizeHalf};
    quads[3] = {{center.x - sizeHalf, center.y + sizeHalf}, sizeHalf};
    quads[2] = {{center.x + sizeHalf, center.y - sizeHalf}, sizeHalf};

    return quads;
  }

  size_t getIdx(const sf::Vector2f& pos) {
    int x = pos.x < center.x;
    int y = pos.y < center.y;

    return y << 1 | x;
  }
};

