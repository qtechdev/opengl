#include <optional>
#include <vector>

#include "glm/vec2.hpp"

#include "collision.hpp"

bool intersectAABB(
  const glm::vec2 &p_pos, const glm::vec2 &p_size,
  const glm::vec2 &q_pos, const glm::vec2 &q_size,
  const double fudge=0
);
int orientation(const glm::vec2 &p, const glm::vec2 &q, const glm::vec2 &r);
bool onSegment(const glm::vec2 &p, const glm::vec2 &r, const glm::vec2 &q) ;
bool intersectLine(
  const glm::vec2 &p_pos, const glm::vec2 &p_next,
  const glm::vec2 &q_pos, const glm::vec2 &q_next
);

/*****************************************************************************/
glm::vec2 lerp(const glm::vec2 &a, const glm::vec2 &b, const double x) {
  glm::vec2 d = b * glm::vec2(x);
  return a + d;
}

std::optional<std::vector<Point *>> checkCollisions(
  Point &p, std::vector<Point> &points
) {
  std::vector<Point *> collided;

  for (auto &q : points) {
    if (p == q) {
      continue;
    }

    if (
      intersectLine(p.position, p.next_position, q.position, q.next_position)
    ) {
     collided.push_back(&q);
    }
  }

  if (collided.size() != 0) {
    return {collided};
  }

  return {};
}


std::optional<std::vector<AABB *>> checkCollisions(
  const AABB &p, std::vector<AABB> &aabbs
) {
  std::vector<AABB *> collided;

  const double fudge = 1;
  for (AABB &q : aabbs) {
    if (p == q) {
      continue;
    }

    if (
      intersectAABB(p.position, p.size, q.position, q.size, fudge) ||
      intersectAABB(
        lerp(p.position, p.next_position, 0.25), p.size,
        lerp(q.position, q.next_position, 0.25), q.size,
        fudge
      ) ||
      intersectAABB(
        lerp(p.position, p.next_position, 0.50), p.size,
        lerp(q.position, q.next_position, 0.50), q.size,
        fudge
      ) ||
      intersectAABB(
        lerp(p.position, p.next_position, 0.75), p.size,
        lerp(q.position, q.next_position, 0.75), q.size,
        fudge
      ) ||
      intersectAABB(p.next_position, p.size, q.next_position, q.size, fudge)
    ) {
     collided.push_back(&q);
    }
  }

  if (collided.size() != 0) {
    return {collided};
  }

  return {};
}

/*****************************************************************************/
bool intersectAABB(
  const glm::vec2 &p_pos, const glm::vec2 &p_size,
  const glm::vec2 &q_pos, const glm::vec2 &q_size,
  const double fudge
) {
  return (
    // p's left edge is to the left of q's right edge
    p_pos.x < (q_pos.x + q_size.x) &&
    // p's right edge is to the right of q's left edge
    (p_pos.x + p_size.x) > q_pos.x &&
    // p's bottom edge is to the bottom of q's top edge
    p_pos.y < (q_pos.y + q_size.y) &&
    // p's top edge is to the top of q's bottom edge
    (p_pos.y + p_size.y) > q_pos.y
  );

}

int orientation(const glm::vec2 &p, const glm::vec2 &q, const glm::vec2 &r) {
  /*
  0 = co-linear
  1 = clockwise
  -1 = anti-clockwise
  */
  int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

  if (val == 0) { return 0; }
  return (val > 0) ? 1 : -1;
}

bool onSegment(const glm::vec2 &p, const glm::vec2 &r, const glm::vec2 &q) {
  return (
    (r.x <= std::max(p.x, q.x)) &&
    (r.x >= std::min(p.x, q.x)) &&
    (r.y <= std::max(p.y, q.y)) &&
    (r.y >= std::min(p.y, q.y))
  );
}

bool intersectLine(
  const glm::vec2 &p_pos, const glm::vec2 &p_next,
  const glm::vec2 &q_pos, const glm::vec2 &q_next
) {
  /*
  1. General Case:
  – (p1, p2, q1) and (p1, p2, q2) have different orientations
  – (q1, q2, p1) and (q1, q2, p2) have different orientations

  2. Special Case
  – (p1, p2, q1), (p1, p2, q2), (q1, q2, p1), and (q1, q2, p2) are all co-linear
  – the x-projections of (p1, p2) and (q1, q2) intersect
  – the y-projections of (p1, p2) and (q1, q2) intersect
  */
  const int o1 = orientation(p_pos, p_next, q_pos);
  const int o2 = orientation(p_pos, p_next, q_next);
  const int o3 = orientation(q_pos, q_next, p_pos);
  const int o4 = orientation(q_pos, q_next, p_next);

  return (
    // General case
    (o1 != o2 && o3 != o4) ||
    // Special cases
    (o1 == 0 && onSegment(p_pos, q_pos, p_next)) ||
    (o2 == 0 && onSegment(p_pos, q_next, p_next)) ||
    (o3 == 0 && onSegment(q_pos, p_pos, q_next)) ||
    (o4 == 0 && onSegment(q_pos, p_next, q_next))
  );
}
