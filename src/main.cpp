#include "ImGuiProfilerRenderer.h"
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

  legit::ProfilerTask profilerTasks[3];
  {
    using namespace legit::Colors;

    profilerTasks[0] = {0, 0, "Quadtree::insert", turqoise};
    profilerTasks[1] = {0, 0, "Quadtree::propagate", emerald};
    profilerTasks[2] = {0, 0, "Qt Acceleration", peterRiver};
  }

  ImGuiUtils::ProfilerGraph profilerGraph(144);

  Spawner::spiral(bodies, BODIES - 1);
  bodies.push_back(Body({WIDTH * 0.5f, HEIGHT * 0.5f},  1e3f));

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

    if (clockTitle.getElapsedTime().asSeconds() > 0.3f) {
      window.setTitle(std::format("FPS: {}, ms: {:.5f}", static_cast<u32>(1.f / dt), dt));
      clockTitle.restart();
    }

    Quad rootQuad = Quad::calcRootQuad(bodies);
    qt.clear(rootQuad);

    profilerTasks[0].startTime = clockMain.getElapsedTime().asSeconds();
    for (const Body& body : bodies) {
      qt.insert(body);
    }
    profilerTasks[0].endTime = clockMain.getElapsedTime().asSeconds();

    profilerTasks[1].startTime = clockMain.getElapsedTime().asSeconds();
    qt.propagate();
    profilerTasks[1].endTime = clockMain.getElapsedTime().asSeconds();

    profilerTasks[2].startTime = clockMain.getElapsedTime().asSeconds();
    for (size_t i = 0; i < BODIES - 1; i++) {
      Body& body = bodies[i];
      sf::Vector2f acc = qt.acceleration(body.pos, 0.5f, 1.f);
      body.update(acc, dt);
      vertices[i].position = body.pos;
    }
    profilerTasks[2].endTime = clockMain.getElapsedTime().asSeconds();

    // ImGui::Begin("Hello, world!");
    // ImGui::Button("Look at this pretty button");
    // ImGui::End();

    profilerGraph.LoadFrameData(profilerTasks, 3);
    profilerGraph.RenderTimings(400, 100, 200, 10, 1.f);

    window.clear();
    window.draw(vertices, states);
    ImGui::SFML::Render(window);
    window.display();
  }

  ImGui::SFML::Shutdown();
}

