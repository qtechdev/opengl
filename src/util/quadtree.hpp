#ifndef __QUADTREE_HPP__
#define __QUADTREE_HPP__

#include <memory>
#include <vector>

namespace quadtree {
  struct Point {
    double x;
    double y;
    void *user_data;
  };

  struct AABB {
    double x;
    double y;
    double half_w;
    double half_h;
  };

  bool contains(const Point &p, const AABB &a);

  class quadtree {
  public:
    quadtree() = default;
    quadtree(const AABB &boundary, const int capacity)
    : boundary(boundary), capacity(capacity) {}

    void clear();
    bool insert(const Point &o);

    std::vector<AABB> getBoundaries();
  private:
    void divide();

    bool is_leaf=true;
    int capacity;
    AABB boundary;
    std::vector<Point> objects;

    std::unique_ptr<quadtree> q1; // upper-right
    std::unique_ptr<quadtree> q2; // upper-left
    std::unique_ptr<quadtree> q3; // lower-left
    std::unique_ptr<quadtree> q4; // lower-right
  };
}

#endif // __QUADTREE_HPP__
