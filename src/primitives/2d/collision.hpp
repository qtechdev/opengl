#ifndef __2D_COLLISION_HPP__
#define __2D_COLLISION_HPP__
#include <optional>
#include <vector>

#include "glm/vec2.hpp"

#include "aabb.hpp"
#include "point.hpp"


glm::vec2 lerp(const glm::vec2 &a, const glm::vec2 &b, const double x);

std::optional<std::vector<AABB *>> checkCollisions(
  const AABB &p, std::vector<AABB> &aabbs
);

std::optional<std::vector<Point *>> checkCollisions(
  Point &p, std::vector<Point> &points
);

#endif // __2D_COLLISION_HPP__
