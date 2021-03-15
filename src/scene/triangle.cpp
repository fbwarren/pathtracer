#include "triangle.h"

#include "CGL/CGL.h"
#include "GL/glew.h"

namespace CGL {
namespace SceneObjects {

Triangle::Triangle(const Mesh *mesh, size_t v1, size_t v2, size_t v3) {
  p1 = mesh->positions[v1];
  p2 = mesh->positions[v2];
  p3 = mesh->positions[v3];
  n1 = mesh->normals[v1];
  n2 = mesh->normals[v2];
  n3 = mesh->normals[v3];
  bbox = BBox(p1);
  bbox.expand(p2);
  bbox.expand(p3);

  bsdf = mesh->get_bsdf();
}

BBox Triangle::get_bbox() const { return bbox; }

bool Triangle::has_intersection(const Ray &r) const {
  // Part 1, Task 3: implement ray-triangle intersection
  // The difference between this function and the next function is that the next
  // function records the "intersection" while this function only tests whether
  // there is a intersection.

  Vector3D intersect = mollerTrumbore(r);
  if (intersect[0] < 0 or intersect[0] < r.min_t or intersect[0] > r.max_t)
    { return false; }
  if (intersect[1] < 0 or intersect[1] > 1 or intersect[2] < 0 or intersect[2] > 1 or intersect[1]+intersect[2] > 1)
    { return false; }
  r.max_t = intersect[0];
  return true;
}

bool Triangle::intersect(const Ray &r, Intersection *isect) const {
  // Part 1, Task 3:
  // implement ray-triangle intersection. When an intersection takes
  // place, the Intersection data should be updated accordingly

    Vector3D intersect = mollerTrumbore(r);
    if (intersect[0] < 0 or intersect[0] < r.min_t or intersect[0] > r.max_t)
        { return false; }
    if (intersect[1] < 0 or intersect[1] > 1 or intersect[2] < 0 or intersect[2] > 1 or intersect[1]+intersect[2] > 1 )
        { return false; }
    r.max_t = intersect[0];
    isect->t = intersect[0];
    isect->n = (1-intersect[1]-intersect[2])*n1 + intersect[1]*n2 + intersect[2]*n3;
    isect->primitive = this;
    isect->bsdf = get_bsdf();

    return true;
}

Vector3D Triangle::mollerTrumbore(const Ray &r) const {
    Vector3D e1, e2, s, s1, s2;
    e1 = p2 - p1;
    e2 = p3 - p1;
    s = r.o - p1;
    s1 = cross(r.d, e2);
    s2 = cross(s, e1);
    Vector3D ret = Vector3D();
    return (1 / dot(s1, e1)) * Vector3D(dot(s2, e2), dot(s1, s), dot(s2, r.d));
}

void Triangle::draw(const Color &c, float alpha) const {
  glColor4f(c.r, c.g, c.b, alpha);
  glBegin(GL_TRIANGLES);
  glVertex3d(p1.x, p1.y, p1.z);
  glVertex3d(p2.x, p2.y, p2.z);
  glVertex3d(p3.x, p3.y, p3.z);
  glEnd();
}

void Triangle::drawOutline(const Color &c, float alpha) const {
  glColor4f(c.r, c.g, c.b, alpha);
  glBegin(GL_LINE_LOOP);
  glVertex3d(p1.x, p1.y, p1.z);
  glVertex3d(p2.x, p2.y, p2.z);
  glVertex3d(p3.x, p3.y, p3.z);
  glEnd();
}

} // namespace SceneObjects
} // namespace CGL
