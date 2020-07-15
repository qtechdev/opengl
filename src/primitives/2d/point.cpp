#include <optional>
#include <vector>

#include "glm/glm.hpp"

#include "point.hpp"

int Point::current_id = 0;

Point::Point() {
  id = Point::current_id;
  Point::current_id++;
}

bool Point::operator==(const Point &rhs) const {
  return id == rhs.id;
}

void updateVelocity(Point &p, const double dt) {
  p.velocity += p.acceleration * glm::vec2(dt);
  p.acceleration = glm::vec2(0.0);

  p.next_position = p.position + (p.velocity * glm::vec2(dt));
}

void updatePosition(Point &p, const double dt) {
  p.position = p.next_position;
}
