#include "ProfilerManager.hpp"
#include "Spawner.hpp"
#include "imgui-SFML.h"

#include "imgui.h"
#include "quadtree/Quadtree.hpp"
#include "utils/utils.hpp"

int main() {
  srand(time(NULL));

  sf::RenderWindow window = sf::RenderWindow(sf::VideoMode({WIDTH, HEIGHT}), "NBody sim");
  window.setFramerateLimit(144);

  if (!ImGui::SFML::Init(window))
    error("ImGui init error");

  std::vector<Body> bodies;
  Quadtree qt;

  sf::Clock clockMain;
  sf::Clock clockTitle;
  float dt;

  sf::RenderStates states;
  states.blendMode = sf::BlendAdd;

  ProfilerManager profilerManager(3, 144);
  sf::VertexArray vertices;
  size_t bodiesCount = 0;
  int initBodiesSpawnCount = 5e4;
  float initCenterBodySpawnMass = 1e4f;

  const auto initSpawn = [&]() {
    bodies.clear();
    Spawner::spiral(bodies, initBodiesSpawnCount);
    bodies.push_back(Body({WIDTH * 0.5f, HEIGHT * 0.5f}, initCenterBodySpawnMass));

    bodiesCount = bodies.size();
    vertices = sf::VertexArray{sf::PrimitiveType::Points, bodiesCount};

    sf::Vertex& lastVertex = vertices[bodiesCount - 1];
    lastVertex.position = {WIDTH * 0.5f, HEIGHT * 0.5f};
    lastVertex.color = sf::Color::Magenta;

    // Set default color
    for (size_t i = 0; i < bodiesCount - 1; i++) {
      vertices[i].color = sf::Color(30, 30, 30);
    }
  };

  initSpawn();

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
          case sf::Keyboard::Scancode::R:
            initSpawn();
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

      for (size_t i = 0; i < bodiesCount - 1; i++) {
        Body& body = bodies[i];
        sf::Vector2f acc = qt.acceleration(body.pos, 0.5f, 1.f);
        body.update(acc, dt);
        vertices[i].position = body.pos;
      }

    }, "Qt Acceleration");

    if (ImGui::TreeNode("Profiler")) {
      profilerManager.render(400, 100, 200, 1, 0.100f);
      ImGui::TreePop();
    }

    ImGui::Spacing();
    ImGui::Text("Bodies: %zu", bodiesCount);
    ImGui::SliderInt("Init bodies spawn count", &initBodiesSpawnCount, 1, 1e6);
    ImGui::SliderFloat("Init Center's mass", &initCenterBodySpawnMass, 0.1f, 1e6f);
    ImGui::SliderFloat("Center's mass", &bodies.back().mass, 0.1f, 1e6f);
    ImGui::SliderFloat("Horizontal acceleration min", &qt.limitX[0], -1000.f, 0.f);
    ImGui::SliderFloat("Horizontal acceleration max", &qt.limitX[1], 0.f, 1000.f);
    ImGui::SliderFloat("Vertical acceleration min", &qt.limitY[0], -1000.f, 0.f);
    ImGui::SliderFloat("Vertical acceleration max", &qt.limitY[1], 0.f, 1000.f);

    window.clear();
    window.draw(vertices, states);
    ImGui::SFML::Render(window);
    window.display();
  }

  ImGui::SFML::Shutdown();
}

