#include "ProfilerManager.hpp"
#include "Spawner.hpp"
#include "imgui-SFML.h"

#include "quadtree/Quadtree.hpp"
#include "utils/utils.hpp"

#define BODIES 50000u

int main() {
  srand(time(NULL));

  sf::RenderWindow window = sf::RenderWindow(sf::VideoMode({WIDTH, HEIGHT}), "NBody sim");
  window.setFramerateLimit(144);

  if (!ImGui::SFML::Init(window))
    error("ImGui init error");

  std::vector<Body> bodies;
  sf::VertexArray vertices{sf::PrimitiveType::Points, BODIES};
  Quadtree qt;

  sf::Clock clockMain;
  sf::Clock clockTitle;
  float dt;

  sf::RenderStates states;
  states.blendMode = sf::BlendAdd;

  ProfilerManager profilerManager(3, 144);

  Spawner::spiral(bodies, BODIES);

  // Set default color
  for (size_t i = 0; i < BODIES; i++) {
    vertices[i].color = sf::Color(30, 30, 30);
  }

  while (window.isOpen()) {
    while (const std::optional event = window.pollEvent()) {
      ImGui::SFML::ProcessEvent(window, *event);

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

    ImGui::SFML::Update(window, clockMain.getElapsedTime());

    dt = clockMain.restart().asSeconds();

    float ms = clockTitle.getElapsedTime().asMilliseconds();
    if (ms > 300.f) {
      window.setTitle(std::format("FPS: {}, ms: {:.0f}", static_cast<u32>(1.f / dt), dt * 1000.f));
      clockTitle.restart();
    }

    Quad rootQuad = Quad::calcRootQuad(bodies);
    qt.clear(rootQuad);

    profilerManager.updateTask(0, [&bodies, &qt] () {

      for (const Body& body : bodies) {
        qt.insert(body);
      }

    }, "Quadtree::insert");

    profilerManager.updateTask(1, [&qt] () {

      qt.propagate();

    }, "Quadtree::propagate");

    profilerManager.updateTask(2, [&] () {

      for (size_t i = 0; i < BODIES; i++) {
        Body& body = bodies[i];
        sf::Vector2f acc = qt.acceleration(body.pos, 0.5f, 1.f);
        body.update(acc, dt);
        vertices[i].position = body.pos;
      }

    }, "Qt Acceleration");

    profilerManager.render(400, 100, 200, 1, 0.100f);

    window.clear();
    window.draw(vertices, states);
    ImGui::SFML::Render(window);
    window.display();
  }

  ImGui::SFML::Shutdown();
}

