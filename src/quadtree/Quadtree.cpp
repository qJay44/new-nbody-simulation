#include "Quadtree.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>

#define SUBDIVIDE_LIMIT 2 << 8

size_t Quadtree::subdivide(size_t node) {
  parents.push_back(node);
  size_t children = nodes.size();
  nodes[node].children = children;

  size_t nexts[4] = {
    children + 1,
    children + 2,
    children + 3,
    nodes[node].next
  };

  const std::array<Quad, 4> quads = nodes[node].quad.intoQuadrants();
  for (size_t i = 0; i < 4; i++) {
    nodes.push_back(Node(quads[i], nexts[i]));
  }

  return children;
}

void Quadtree::insert(const Body& body) {
  size_t nodeIdx = 0;

  while (nodes[nodeIdx].isBranch()) {
    size_t quadIdx = nodes[nodeIdx].quad.getIdx(body.pos);
    nodeIdx = nodes[nodeIdx].children + quadIdx;
  }

  if (nodes[nodeIdx].isEmpty()) {
    nodes[nodeIdx].massCenter = body.pos;
    nodes[nodeIdx].mass = body.mass;
    return;
  };

  sf::Vector2f currMassCenter = nodes[nodeIdx].massCenter;
  float currMass = nodes[nodeIdx].mass;

  if (currMassCenter == body.pos) {
    nodes[nodeIdx].mass += body.mass;
    return;
  }

  while (true) {
    static int i = 0;

    size_t children = subdivide(nodeIdx);
    size_t quadIdx1 = nodes[nodeIdx].quad.getIdx(currMassCenter);
    size_t quadIdx2 = nodes[nodeIdx].quad.getIdx(body.pos);

    if (quadIdx1 == quadIdx2 && (++i < SUBDIVIDE_LIMIT)) {
      nodeIdx = children + quadIdx1;
    } else {
      size_t nodeIdx1 = children + quadIdx1;
      size_t nodeIdx2 = children + quadIdx2;

      nodes[nodeIdx1].massCenter = currMassCenter;
      nodes[nodeIdx1].mass = currMass;
      nodes[nodeIdx2].massCenter = body.pos;
      nodes[nodeIdx2].mass = body.mass;
      return;
    }
  }
}

void Quadtree::propagate() {
  for (auto it = parents.rbegin(); it != parents.rend(); it++) {
    Node& node = nodes[*it];

    size_t i = node.children;

    node.massCenter = nodes[i + 0].massCenter * nodes[i + 0].mass +
                      nodes[i + 1].massCenter * nodes[i + 1].mass +
                      nodes[i + 2].massCenter * nodes[i + 2].mass +
                      nodes[i + 3].massCenter * nodes[i + 3].mass;

    node.mass = nodes[i].mass + nodes[i + 1].mass + nodes[i + 2].mass + nodes[i + 3].mass;
    node.massCenter /= node.mass;
  }
}

sf::Vector2f Quadtree::acceleration(sf::Vector2f pos, float theta, float epsilon) const {
  sf::Vector2f acc{0.f, 0.f};

  float thetaSq = theta * theta;
  float epsilonSq = epsilon * epsilon;
  size_t nodeIdx = 0;

  while (true) {
    #ifdef DEBUG
      constexpr size_t _loopLimit = 1e6;
      static size_t i = 0;
      assert(i++ < _loopLimit);
    #endif

    const Node& node = nodes[nodeIdx];
    sf::Vector2f dstVec = node.massCenter - pos;
    float dstSq = dstVec.lengthSquared();
    float quadSizeSq = node.quad.size * node.quad.size;

    if (!node.isBranch() || quadSizeSq < dstSq * thetaSq) {
      float denom = (dstSq + epsilonSq) * std::sqrt(dstSq);
      acc += dstVec * std::min((node.mass / denom), FLT_MAX);

      if (node.next == 0)
        break;

      nodeIdx = node.next;
    } else {
      nodeIdx = node.children;
    }
  }

  return acc;
}

void Quadtree::clear(const Quad& rootQuad) {
  parents.clear();
  nodes.clear();
  nodes.push_back(Node(rootQuad, 0));
}

