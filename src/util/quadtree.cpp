#include "quadtree.hpp"

bool quadtree::contains(const Point &p, const AABB &a) {
  return (
    // p is to the left of a's right edge
    (p.x < (a.x + a.half_w)) &&
    // p is to the right (or on) of a's left edge
    (p.x >= (a.x - a.half_w)) &&
    // p is below a's top edge
    (p.y < (a.y + a.half_h)) &&
    // p is above (or on) a's bottom edge
    (p.y >= (a.y - a.half_h))
  );
}

void quadtree::quadtree::clear() {
  *this = {boundary, capacity};
}

bool quadtree::quadtree::insert(const Point &o) {
  if (!contains(o, boundary)) {
    return false;
  }

  if (is_leaf) {
    objects.push_back(o);
  } else {
    q1->insert(o);
    q2->insert(o);
    q3->insert(o);
    q4->insert(o);
  }

  if (objects.size() > capacity) {
    divide();
    is_leaf = false;
    objects.clear();
  }

  return true;
}

std::vector<quadtree::Point> quadtree::quadtree::search(const AABB &area) {
  std::vector<Point> points;
  if (is_leaf) {
    for (const auto &o : objects) {
      if (contains(o, area)) {
        points.push_back(o);
      }
    }

    return points;
  }

  for (const auto &o : q1->search(area)) {
    points.push_back(o);
  }
  for (const auto &o : q2->search(area)) {
    points.push_back(o);
  }
  for (const auto &o : q3->search(area)) {
    points.push_back(o);
  }
  for (const auto &o : q4->search(area)) {
    points.push_back(o);
  }

  return points;
}

std::vector<quadtree::AABB> quadtree::quadtree::getBoundaries() {
  if (is_leaf) {
    return {boundary};
  }

  std::vector<AABB> boundaries = {boundary};

  for (const auto &b : q1->getBoundaries()) {
    boundaries.push_back(b);
  }
  for (const auto &b : q2->getBoundaries()) {
    boundaries.push_back(b);
  }
  for (const auto &b : q3->getBoundaries()) {
    boundaries.push_back(b);
  }
  for (const auto &b : q4->getBoundaries()) {
    boundaries.push_back(b);
  }

  return boundaries;
}

void quadtree::quadtree::divide() {
  const auto [x, y, half_w, half_h] = boundary;

  double half_x = x / 2.0;
  double half_y = y / 2.0;
  double quarter_w = half_w / 2.0;
  double quarter_h = half_h / 2.0;

  AABB q1_aabb = {x + quarter_w, y + quarter_h, quarter_w, quarter_h};
  AABB q2_aabb = {x - quarter_w, y + quarter_h, quarter_w, quarter_h};
  AABB q3_aabb = {x - quarter_w, y - quarter_h, quarter_w, quarter_h};
  AABB q4_aabb = {x + quarter_w, y - quarter_h, quarter_w, quarter_h};

  q1 = std::make_unique<quadtree>(q1_aabb, capacity);
  q2 = std::make_unique<quadtree>(q2_aabb, capacity);
  q3 = std::make_unique<quadtree>(q3_aabb, capacity);
  q4 = std::make_unique<quadtree>(q4_aabb, capacity);

  for (const auto &o : objects) {
    q1->insert(o);
    q2->insert(o);
    q3->insert(o);
    q4->insert(o);
  }
}
