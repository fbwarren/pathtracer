#include "bvh.h"

#include "CGL/CGL.h"
#include "triangle.h"

#include <iostream>
#include <stack>

using namespace std;

namespace CGL {
namespace SceneObjects {

BVHAccel::BVHAccel(const std::vector<Primitive *> &_primitives,
                   size_t max_leaf_size) {

  primitives = std::vector<Primitive *>(_primitives);
  root = construct_bvh(primitives.begin(), primitives.end(), max_leaf_size);
}

BVHAccel::~BVHAccel() {
  if (root)
    delete root;
  primitives.clear();
}

BBox BVHAccel::get_bbox() const { return root->bb; }

void BVHAccel::draw(BVHNode *node, const Color &c, float alpha) const {
  if (node->isLeaf()) {
    for (auto p = node->start; p != node->end; p++) {
      (*p)->draw(c, alpha);
    }
  } else {
    draw(node->l, c, alpha);
    draw(node->r, c, alpha);
  }
}

void BVHAccel::drawOutline(BVHNode *node, const Color &c, float alpha) const {
  if (node->isLeaf()) {
    for (auto p = node->start; p != node->end; p++) {
      (*p)->drawOutline(c, alpha);
    }
  } else {
    drawOutline(node->l, c, alpha);
    drawOutline(node->r, c, alpha);
  }
}

BVHNode *BVHAccel::construct_bvh(std::vector<Primitive *>::iterator start,
                                 std::vector<Primitive *>::iterator end,
                                 size_t max_leaf_size) {
  // box enclosing all primitives
  BBox bbox;
  for (auto p = start; p != end; p++) {
      BBox bb = (*p)->get_bbox();
      bbox.expand(bb);
  }

  auto *node = new BVHNode(bbox);    // initialize node with bounding box (L & R are null)
  node->start = start;
  node->end = end;

  // base case (node doesn't contain more than max allowed)
  int size = distance(start, end);
  if (size <= max_leaf_size) {
      return node;
  }

  // compute longest axis
  Vector3D extent = bbox.extent;
  int axis = (extent[0] > extent[1]) ? 0 : 1;
  axis = (extent[axis] > extent[2]) ? axis : 2;

  // compute average centroid
  Vector3D sum = Vector3D(.0, .0, .0);
  for (auto p = start; p != end; ++p)
      sum += (*p)->get_bbox().centroid();
  sum /= size;

  // split along centroid on chosen axis (split plane)
  auto *left = new vector<Primitive *>();
  auto *right = new vector<Primitive *>();
  auto func = [&](Primitive *p) {
      if (p->get_bbox().centroid()[axis] < sum[axis])
          left->push_back(p);
      else
          right->push_back(p);
  };
  for_each(start, end, func);

  // make sure both subsets aren't empty
  if (left->empty()) {
      left->push_back(right->back());
      right->pop_back();
  } else if (right->empty()) {
      right->push_back(left->back());
      left->pop_back();
  }

  node->l = construct_bvh(left->begin(), left->end(), max_leaf_size);
  node->r = construct_bvh(right->begin(), right->end(), max_leaf_size);

  return node;
}

bool BVHAccel::has_intersection(const Ray &ray, BVHNode *node) const {
  bool hit = false;
  double t0 = ray.min_t, t1 = ray.max_t;

  // check ray bounding box intersection
  if (node->bb.intersect(ray, t0, t1)) {
      // check ray intersection with primitives once at a leaf node
      if (t0 >= ray.min_t && t0 <= ray.max_t && t1 >= ray.min_t && t1 <= ray.max_t) {
          if (node->isLeaf()) {
              for (auto p = node->start; p != node->end && !hit; ++p) {
                  total_isects++;
                  hit = (*p)->has_intersection(ray) || hit;
              }
          } else {      // if not at leaf node, recurse down tree
              hit = (has_intersection(ray, node->l) || has_intersection(ray, node->r));
          }
      }
  }
  return hit;
}

bool BVHAccel::intersect(const Ray &ray, Intersection *i, BVHNode *node) const {
  bool hit = false;
  double t0 = ray.min_t, t1 = ray.max_t;

  // check ray bounding box intersection
  if (node->bb.intersect(ray, t0, t1)) {
      // check ray intersection with primitives once at a leaf node
      if (t0 >= ray.min_t && t0 <= ray.max_t && t1 >= ray.min_t && t1 <= ray.max_t) {
          if (node->isLeaf()) {
              for (auto p = node->start; p != node->end; ++p) {
                  total_isects++;
                  hit = (*p)->intersect(ray, i) || hit;
              }
          } else {      // if not at leaf node, recurse down tree
              hit |= intersect(ray, i, node->l);
              hit |= intersect(ray, i, node->r);
          }
      }
  }
  return hit;
}

} // namespace SceneObjects
} // namespace CGL
