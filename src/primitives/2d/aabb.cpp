#include <optional>
#include <vector>

#include "glm/glm.hpp"

#include "aabb.hpp"

int AABB::current_id = 0;

AABB::AABB() {
  id = AABB::current_id;
  AABB::current_id++;
}

bool AABB::operator==(const AABB &rhs) const {
  return id == rhs.id;
}

void updateVelocity(AABB &p, const double dt) {
  p.velocity += p.acceleration * glm::vec2(dt);
  p.acceleration = glm::vec2(0.0);

  p.next_position = p.position + (p.velocity * glm::vec2(dt));
}

void updatePosition(AABB &p, const double dt) {
  p.position = p.next_position;
}
