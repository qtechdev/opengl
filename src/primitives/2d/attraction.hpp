#ifndef __2D_ATTRACTION_HPP__
#define __2D_ATTRACTION_HPP__

#include <vector>

#include "aabb.hpp"
#include "point.hpp"

double dist_sq(const AABB &p, const AABB &q);

double dist_sq(const Point &p, const Point &q);

void attract(
  AABB &p, const std::vector<AABB *> aabbs,
  const double min_dist=0, const double max_dist=0
);

void attract(
  Point &p, const std::vector<Point *> points,
  const double min_dist=0, const double max_dist=0
);

#endif // __2D_ATTRACTION_HPP__
