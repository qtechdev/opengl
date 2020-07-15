#include <memory>
#include <vector>

#include "point.hpp"
#include "attraction.hpp"

double dist_sq(const AABB &p, const AABB &q) {
  // boxes; left, right, bottom, top
  AABB lb = (p.position.x < q.position.x) ? p : q;
  AABB rb = (lb == q) ? p : q;
  AABB bb = (p.position.y < q.position.y) ? p : q;
  AABB tb = (bb == q) ? p : q;

  // extents; right, left, top, bottom
  double re = lb.position.x + lb.size.x;
  double le = rb.position.x;
  double te = bb.position.y + bb.size.y;
  double be = tb.position.y;

  double dx = re - le;
  double dy = te - be;
  return (dx * dx) + (dy * dy);
}

double dist_sq(const Point &p, const Point &q) {
  double dx = p.position.x - q.position.x;
  double dy = p.position.y - q.position.y;
  return (dx * dx) + (dy * dy);
}

void attract(
  AABB &p, const std::vector<std::shared_ptr<AABB>> &aabbs,
  const double min_dist, const double max_dist
) {
  const double min_dist_sq = min_dist * min_dist;
  const double max_dist_sq = max_dist * max_dist;

  for (const std::shared_ptr<AABB> q : aabbs) {
    if (p == *q) {
      continue;
    }

    double dsq = dist_sq(p, *q);
    if (
      ((min_dist_sq < max_dist_sq) && (dsq < min_dist_sq)) ||
      ((max_dist_sq != 0) && (dsq > max_dist_sq)) ||
      (dsq == 0)
    ) {
      continue;
    }

    glm::vec2 direction = glm::normalize(q->position - p.position);
    double magnitude = (p.mass * q->mass) / dsq;
    glm::vec2 force = direction * glm::vec2(magnitude);

    p.acceleration += force / glm::vec2(p.mass);
  }
}

void attract(
  Point &p, const std::vector<Point> &points,
  const double min_dist, const double max_dist
) {
  const double min_dist_sq = min_dist * min_dist;
  const double max_dist_sq = max_dist * max_dist;

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
