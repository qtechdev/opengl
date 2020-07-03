#ifndef __POINT_HPP__
#define __POINT_HPP__
#include <optional>
#include <vector>

#include "glm/glm.hpp"

struct Point {
  Point();
  static int current_id;
  int id;

  glm::vec2 position={0.0, 0.0};
  glm::vec2 next_position={0.0, 0.0};
  glm::vec2 size={1.0, 1.0};

  glm::vec2 velocity={0.0, 0.0};
  glm::vec2 acceleration={0.0, 0.0};
  double mass=1;
  bool is_alive=true;

  bool operator==(const Point &rhs) const;
  bool operator!=(const Point &rhs) const { return !(*this == rhs); }
};

double dist_sq(const Point &p, const Point &q);

void attract(
  Point &p, const std::vector<Point> &points,
  const double min_dist_sq=0, const double max_dist_sq=0
);
void updateVelocity(Point &p, const double dt);
void updatePosition(Point &p, const double dt);

std::optional<std::vector<Point *>> checkCollisions(
  Point &p, std::vector<Point> &points
);

#endif // __POINT_HPP__
