#include "quadtree/Quadtree.hpp"

#define BODIES 100000

int main() {
  srand(time(NULL));

  sf::RenderWindow window = sf::RenderWindow(sf::VideoMode({WIDTH, HEIGHT}), "NBody sim");
  window.setFramerateLimit(144);

  std::vector<Body> bodies;
  sf::VertexArray vertices{sf::PrimitiveType::Points, BODIES};
  Quadtree qt;

  sf::Clock clockMain;
  sf::Clock clockTitle;
  float dt;
  sf::Vector2f center = sf::Vector2f(WIDTH, HEIGHT) * 0.5f;

  sf::RenderStates states;
  states.blendMode = sf::BlendAdd;

  // Set default color for vertices
  for (size_t i = 0; i < BODIES; i++) {
    vertices[i].color = sf::Color(30, 30, 30);
  }

  // Spawn particles
  for (size_t i = 0; i < BODIES; i++) {
    constexpr int spawnRadius = HEIGHT * 0.48f;
    constexpr int max =  1000;
    constexpr int min = -1000;

    sf::Vector2f dir(
      (rand() % (max - min + 1) + min) / (float)max,
      (rand() % (max - min + 1) + min) / (float)max
    );
    dir = dir.normalized();

    float dist = rand() % spawnRadius;

    bodies.push_back({center + dir * dist, 1.f});
  }

  while (window.isOpen()) {
    while (const std::optional event = window.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        window.close();
      } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
        switch (keyPressed->scancode) {
          case sf::Keyboard::Scancode::Q:
            window.close();
            break;
          default:
            break;
        };
      }
    }

    dt = clockMain.restart().asSeconds();

    if (clockTitle.getElapsedTime().asSeconds() > 0.3f) {
      window.setTitle(std::format("FPS: {}, ms: {:.5f}", static_cast<u32>(1.f / dt), dt));
      clockTitle.restart();
    }

    Quad rootQuad = Quad::calcRootQuad(bodies);
    qt.clear(rootQuad);

    for (const Body& body : bodies) {
      qt.insert(body);
    }

    qt.propagate();

    for (size_t i = 0; i < BODIES; i++) {
      Body& body = bodies[i];
      sf::Vector2f acc = qt.acceleration(body.pos, 1.f, 1.f);
      body.update(acc, dt);

      vertices[i].position = body.pos;
    }

    window.clear();
    window.draw(vertices, states);
    window.display();
  }
}

