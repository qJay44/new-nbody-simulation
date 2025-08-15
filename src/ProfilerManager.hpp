#pragma once

#include <cassert>
#include <chrono>
#include <functional>

#include "ImGuiProfilerRenderer.h"
#include "ProfilerTask.h"

class ProfilerManager {
public:
  ProfilerManager(u8 tasksCount, size_t framesCount) : tasksCount(tasksCount), graph(framesCount) {
    tasks = new legit::ProfilerTask[tasksCount];
  }

  ~ProfilerManager() {
    delete[] tasks;
  }

  void updateTask(size_t i, const std::function<void()>& func, const std::string& name) {
    using namespace std::chrono;
    assert(i < tasksCount);

    legit::ProfilerTask& task = tasks[i];
    task.name = name;
    task.color = getColorBright(i);
    task.startTime = 0.f;

    auto start = steady_clock::now();
    func();
    auto end = steady_clock::now();

    task.endTime = duration_cast<milliseconds>(end - start).count() / 1000.f;
  }

  void render(int graphWidth, int legendWidth, int height, int frameIndexOffset, float maxFrameTime)  {
    graph.LoadFrameData(tasks, tasksCount);
    graph.RenderTimings(graphWidth, legendWidth, height, frameIndexOffset, maxFrameTime);
  }

private:
  u8 tasksCount = 0;
  legit::ProfilerTask* tasks = nullptr;
  ImGuiUtils::ProfilerGraph graph;

private:
  const u32& getColorBright(size_t i) const {
    using namespace legit::Colors;

    static constexpr u32 colors[8] = {
      turqoise, emerald, peterRiver, amethyst, sunFlower, carrot, alizarin, clouds
    };

    return colors[i % 8];
  }

  const u32& getColorDim(size_t i) const {
    using namespace legit::Colors;

    static constexpr u32 colors[8] = {
      greenSea, nephritis, belizeHole, wisteria, orange, pumpkin, pomegranate, silver
    };

    return colors[i % 8];
  }
};

