#include "sphere.h"

#include <cmath>

#include "pathtracer/bsdf.h"
#include "util/sphere_drawing.h"

namespace CGL {
namespace SceneObjects {

bool Sphere::test(const Ray &r, double &t1, double &t2) const {
  double a, b, c, discriminant;
  a = dot(r.d, r.d);
  b = dot(2 * (r.o - o), r.d);
  c = dot((r.o - o), (r.o - o)) - r2;
  discriminant = b*b - 4*a*c;

  // no intersection case
  if (discriminant < 0)
      return false;

  t1 = (-1.0 * b - sqrt(discriminant)) / (2.0 * a);
  t2 = (-1.0 * b + sqrt(discriminant)) / (2.0 * a);
  if (t1 > t2) {        // set t1 as smaller t value
      double t = t1;
      t1 = t2;
      t2 = t;
  }

  return true;
}

bool Sphere::has_intersection(const Ray &r) const {
  double t1, t2;
  if (!test(r, t1, t2))
      return false;

  // set the new max intersection to the smallest valid t value if possible
  if (t1 >= r.min_t and t1 <= r.max_t) {
      r.max_t = t1;
  } else if (t2 >= r.min_t and t2 <= r.max_t) {
      r.max_t = t2;
  } else
      return false;

  return true;
}

bool Sphere::intersect(const Ray &r, Intersection *i) const {
  if (!has_intersection(r))
      return false;

  i->t = r.max_t;
  i->n = normal(r.at_time(r.max_t));  // possibly need a fix later? need to subtract origin?
  i->primitive = this;
  i->bsdf = get_bsdf();

  return true;
}

void Sphere::draw(const Color &c, float alpha) const {
  Misc::draw_sphere_opengl(o, r, c);
}

void Sphere::drawOutline(const Color &c, float alpha) const {
  // Misc::draw_sphere_opengl(o, r, c);
}

} // namespace SceneObjects
} // namespace CGL
