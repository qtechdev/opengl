#ifndef __2D_AABB_HPP__
#define __2D_AABB_HPP__
#include "glm/glm.hpp"

struct AABB {
  AABB();
  static int current_id;
  int id;

  glm::vec2 position={0.0, 0.0};
  glm::vec2 next_position={0.0, 0.0};
  glm::vec2 size={1.0, 1.0};

  glm::vec2 velocity={0.0, 0.0};
  glm::vec2 acceleration={0.0, 0.0};
  double mass=1;
  bool is_alive=true;

  bool operator==(const AABB &rhs) const;
  bool operator!=(const AABB &rhs) const { return !(*this == rhs); }
};

void updateVelocity(AABB &p, const double dt);
void updatePosition(AABB &p, const double dt);

#endif // __2D_AABB_HPP__
