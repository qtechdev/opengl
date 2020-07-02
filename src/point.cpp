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

double dist_sq(const Point &p, const Point &q) {
  const double dx = p.position.x - q.position.x;
  const double dy = p.position.y - q.position.y;
  return (dx * dx) + (dy * dy);
}

void attract(
  Point &p, const std::vector<Point> &points,
  const double min_dist_sq, const double max_dist_sq
) {
  for (const auto &q : points) {
    if (p == q) {
      continue;
    }

    double dsq = dist_sq(p, q);
    if (
      ((min_dist_sq < max_dist_sq) && (dsq < min_dist_sq)) ||
      ((max_dist_sq != 0) && (dsq > max_dist_sq)) ||
      (dsq == 0)
    ) {
      continue;
    }

    glm::vec2 direction = glm::normalize(q.position - p.position);
    double magnitude = (p.mass * q.mass) / dsq;
    glm::vec2 force = direction * glm::vec2(magnitude);

    p.acceleration += force / glm::vec2(p.mass);
  }
}

void updateVelocity(Point &p, const double dt) {
  p.velocity += p.acceleration * glm::vec2(dt);
  p.acceleration = glm::vec2(0.0);

  p.next_position = p.position + (p.velocity * glm::vec2(dt));
}

void updatePosition(Point &p, const double dt) {
  p.position = p.next_position;
}

int orientation(glm::vec2 p, glm::vec2 q, glm::vec2 r) {
  /*
  0 = co-linear
  1 = clockwise
  -1 = anti-clockwise
  */
  int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

  if (val == 0) { return 0; }
  return (val > 0) ? 1 : -1;
}

bool onSegment(glm::vec2 p, glm::vec2 x, glm::vec2 q) {
  return (
    (x.x <= std::max(p.x, q.x)) &&
    (x.x >= std::min(p.x, q.x)) &&
    (x.y <= std::max(p.y, q.y)) &&
    (x.y >= std::min(p.y, q.y))
  );
}

bool checkIntersect(
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

std::optional<std::vector<const Point *>> checkCollisions(
  Point &p, const std::vector<Point> &points
) {
  std::vector<const Point *> collided;

  for (auto &q : points) {
    if (p == q) {
      continue;
    }

    if (
      (dist_sq(p, q) <= 2) ||
      checkIntersect(p.position, p.next_position, q.position, q.next_position)
    ) {
     collided.push_back(&q);
    }
  }

  if (collided.size() != 0) {
    return {collided};
  }

  return {};
}
