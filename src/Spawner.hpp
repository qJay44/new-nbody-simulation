#pragma once

#include <cmath>
#include <vector>

#include "quadtree/Body.hpp"

#define SPIRAL_ARMS 2
#define SPIRAL_ARMS_WIDTH 90          // Mini arms in arms
#define SPIRAL_ARMS_WIDTH_VALUE 1.5f  // Distance between each mini arm (pi divider)
#define SPIRAL_ARM_TWIST_VALUE 100.f  // How much the arm is twisted (pi divider)

struct Spawner {
  static void circle(std::vector<Body>& bodies, size_t count, int radius = HEIGHT * 0.48f, sf::Vector2f center = {WIDTH * 0.5f, HEIGHT * 0.5f}) {
    bodies.clear();
    for (size_t i = 0; i < count; i++) {
      constexpr int max =  1000;
      constexpr int min = -1000;

      sf::Vector2f dir(
        (rand() % (max - min + 1) + min) / (float)max,
        (rand() % (max - min + 1) + min) / (float)max
      );
      dir = dir.normalized();

      float dist = rand() % radius;

      bodies.push_back(Body(center + dir * dist, 1.f));
    }
  }

  static void spiral(std::vector<Body>& bodies, size_t count, sf::Vector2f center = {WIDTH * 0.5f, HEIGHT * 0.5f}) {
    bodies.clear();
    float stepRad = (2.f * PI) / SPIRAL_ARMS;
    int spiralArmLength = count / SPIRAL_ARMS / SPIRAL_ARMS_WIDTH;

    for (int i = 0; i < SPIRAL_ARMS; i++) {
      float startRad = i * stepRad;
      for (int j = 0; j < SPIRAL_ARMS_WIDTH; j++) {
        float startArmRad = j * PI / SPIRAL_ARMS_WIDTH_VALUE / SPIRAL_ARMS_WIDTH;
        for (int k = 0; k < spiralArmLength; k++) {
          sf::Vector2f pos = center;
          float rad = startRad + startArmRad + k * PI / SPIRAL_ARM_TWIST_VALUE;
          sf::Vector2f posOnCircle = {cosf(rad), sinf(rad)};
          pos += posOnCircle * (float)k;
          bodies.push_back(Body(pos, 1.f, -posOnCircle * 10.f));
        }
      }
    }
  }
};

