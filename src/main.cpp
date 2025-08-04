#include "ImGuiProfilerRenderer.h"
#include "imgui-SFML.h"

#include "quadtree/Quadtree.hpp"
#include "utils/utils.hpp"

#define BODIES 50000

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
  sf::Vector2f center = sf::Vector2f(WIDTH, HEIGHT) * 0.5f;

  sf::RenderStates states;
  states.blendMode = sf::BlendAdd;

  legit::ProfilerTask profilerTasks[3];
  {
    using namespace legit::Colors;

    profilerTasks[0] = {0, 0, "Quadtree::insert", turqoise};
    profilerTasks[1] = {0, 0, "Quadtree::propagate", emerald};
    profilerTasks[2] = {0, 0, "Quadtree::acceleration", peterRiver};
  }

  ImGuiUtils::ProfilerGraph profilerGraph(144);

  // Setup particles
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

    for (size_t i = 0; i < BODIES; i++) {
      Body& body = bodies[i];

      profilerTasks[2].startTime = clockMain.getElapsedTime().asSeconds();
      sf::Vector2f acc = qt.acceleration(body.pos, 0.55555f, 1.f);
      profilerTasks[2].endTime = clockMain.getElapsedTime().asSeconds();

      body.update(acc, dt);

      vertices[i].position = body.pos;
    }

    ImGui::Begin("Hello, world!");
    ImGui::Button("Look at this pretty button");
    ImGui::End();

    profilerGraph.LoadFrameData(profilerTasks, 3);
    profilerGraph.RenderTimings(400, 100, 200, 10, 100.f);

    window.clear();
    window.draw(vertices, states);
    ImGui::SFML::Render(window);
    window.display();
  }

  ImGui::SFML::Shutdown();
}

