#include "bbox.h"

#include "GL/glew.h"

#include <algorithm>
#include <iostream>

namespace CGL {

bool BBox::intersect(const Ray& r, double& t0, double& t1) const {

  // TODO (Part 2.2):
  // Implement ray - bounding box intersection test
  // If the ray intersected the bounding box within the range given by
  // t0, t1, update t0 and t1 with the new intersection times.

  // Bbox::min & Bbox::max are points that are in 3 distinct faces each
  // since the faces are axis aligned, we can define the slabs with these points and the standard basis as normals
  double tMin, tMax;
  bool hit = false;
  for (int i=0; i<3; i++) {
      if (r.d[i] == 0) {    // ray won't intersect a plane it's parallel to
          tMin = -1;
          tMax = -1;
      } else {      // update t0 & t1 if intersection time is >= 0 and within [t0, t1]
          tMin = (min[i] - r.o[i]) / r.d[i];
          t0 = (tMin < t0 || tMin < 0) ? t0 : tMin;
          tMax = (min[i] - r.o[i]) / r.d[i];
          t1 = (tMax > t1 ) ? t1 : tMax;
      }
      hit = (hit || (t0 == tMin || t1 == tMax));    // hit pins to true if there's a single hit
  }
  return hit;
}

void BBox::draw(Color c, float alpha) const {

  glColor4f(c.r, c.g, c.b, alpha);

  // top
  glBegin(GL_LINE_STRIP);
  glVertex3d(max.x, max.y, max.z);
  glVertex3d(max.x, max.y, min.z);
  glVertex3d(min.x, max.y, min.z);
  glVertex3d(min.x, max.y, max.z);
  glVertex3d(max.x, max.y, max.z);
  glEnd();

  // bottom
  glBegin(GL_LINE_STRIP);
  glVertex3d(min.x, min.y, min.z);
  glVertex3d(min.x, min.y, max.z);
  glVertex3d(max.x, min.y, max.z);
  glVertex3d(max.x, min.y, min.z);
  glVertex3d(min.x, min.y, min.z);
  glEnd();

  // side
  glBegin(GL_LINES);
  glVertex3d(max.x, max.y, max.z);
  glVertex3d(max.x, min.y, max.z);
  glVertex3d(max.x, max.y, min.z);
  glVertex3d(max.x, min.y, min.z);
  glVertex3d(min.x, max.y, min.z);
  glVertex3d(min.x, min.y, min.z);
  glVertex3d(min.x, max.y, max.z);
  glVertex3d(min.x, min.y, max.z);
  glEnd();

}

std::ostream& operator<<(std::ostream& os, const BBox& b) {
  return os << "BBOX(" << b.min << ", " << b.max << ")";
}

} // namespace CGL
