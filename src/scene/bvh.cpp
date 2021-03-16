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

  BVHNode *node = new BVHNode(bbox);    // initialize node with bounding box (L & R are null)
  node->start = start;
  node->end = end;

  // base case (node doesn't contain more than max allowed)
  int size = distance(start, end);
  if (size < max_leaf_size) {
      return node;
  }

  // compute longest axis
  int axis = 0;
  Vector3D extent = bbox.extent;
  axis = (extent[0] > extent[1]) ? 0 : 1;
  axis = (extent[axis] > extent[2]) ? axis : 2;

  // compute average  centroid
  Vector3D sum = Vector3D(.0, .0, .0);
  for (auto p = start; p != end; p++)
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
      left->push_back(*(right->end()));
      right->pop_back();
  } else if (right->empty()) {
      right->push_back(*(left->end()));
      left->pop_back();
  }

  node->l = construct_bvh(left->begin(), left->end(), max_leaf_size);
  node->r = construct_bvh(right->begin(), right->end(), max_leaf_size);

  return node;
}

bool BVHAccel::has_intersection(const Ray &ray, BVHNode *node) const {
  // TODO (Part 2.3):
  // Fill in the intersect function.
  // Take note that this function has a short-circuit that the
  // Intersection version cannot, since it returns as soon as it finds
  // a hit, it doesn't actually have to find the closest hit.
  for (auto p : primitives) {
    total_isects++;
    if (p->has_intersection(ray))
      return true;
  }
  return false;


}

bool BVHAccel::intersect(const Ray &ray, Intersection *i, BVHNode *node) const {
  // TODO (Part 2.3):
  // Fill in the intersect function.
  bool hit = false;
  for (auto p : primitives) {
    total_isects++;
    hit = p->intersect(ray, i) || hit;
  }
  return hit;


}

} // namespace SceneObjects
} // namespace CGL
