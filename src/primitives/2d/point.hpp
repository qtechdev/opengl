#ifndef __2D_POINT_HPP__
#define __2D_POINT_HPP__

#include "glm/vec2.hpp"

struct Point {
  Point();
  static int current_id;
  int id;

  glm::vec2 position = glm::vec2(0.0);
  glm::vec2 next_position = glm::vec2(0.0);
  const glm::vec2 size = glm::vec2(1.0);

  glm::vec2 velocity = glm::vec2(0.0);
  glm::vec2 acceleration = glm::vec2(0.0);
  double mass = 1.0;

  bool is_alive = true;

  bool operator==(const Point &rhs) const;
  bool operator!=(const Point &rhs) const { return !(*this == rhs); }
};

void updateVelocity(Point &p, const double dt);
void updatePosition(Point &p, const double dt);

#endif // __2D_POINT_HPP__
