#include "pathtracer.h"

#include "scene/light.h"
#include "scene/sphere.h"
#include "scene/triangle.h"


using namespace CGL::SceneObjects;

namespace CGL {

PathTracer::PathTracer() {
  gridSampler = new UniformGridSampler2D();
  hemisphereSampler = new UniformHemisphereSampler3D();

  tm_gamma = 2.2f;
  tm_level = 1.0f;
  tm_key = 0.18;
  tm_wht = 5.0f;
}

PathTracer::~PathTracer() {
  delete gridSampler;
  delete hemisphereSampler;
}

void PathTracer::set_frame_size(size_t width, size_t height) {
  sampleBuffer.resize(width, height);
  sampleCountBuffer.resize(width * height);
}

void PathTracer::clear() {
  bvh = NULL;
  scene = NULL;
  camera = NULL;
  sampleBuffer.clear();
  sampleCountBuffer.clear();
  sampleBuffer.resize(0, 0);
  sampleCountBuffer.resize(0, 0);
}

void PathTracer::write_to_framebuffer(ImageBuffer &framebuffer, size_t x0,
                                      size_t y0, size_t x1, size_t y1) {
  sampleBuffer.toColor(framebuffer, x0, y0, x1, y1);
}

Vector3D
PathTracer::estimate_direct_lighting_hemisphere(const Ray &r,
                                                const Intersection &isect) {
  // Estimate the lighting from this intersection coming directly from a light.
  // For this function, sample uniformly in a hemisphere.

  // Note: When comparing Cornel Box (CBxxx.dae) results to importance sampling, you may find the "glow" around the light source is gone.
  // This is totally fine: the area lights in importance sampling has directionality, however in hemisphere sampling we don't model this behaviour.

  // make a coordinate system for a hit point
  // with N aligned with the Z direction.
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  // w_out points towards the source of the ray (e.g.,
  // toward the camera if this is a primary ray)
  const Vector3D hit_p = r.o + r.d * isect.t;
  const Vector3D w_out = w2o * (-r.d);

  // This is the same number of total samples as
  // estimate_direct_lighting_importance (outside of delta lights). We keep the
  // same number of samples for clarity of comparison.
  int num_samples = scene->lights.size() * ns_area_light;
  Vector3D L_out;

  for (int i=0; i < num_samples; ++i) {
      // sample uniformly, this is in object space
      Vector3D w_in = hemisphereSampler->get_sample(), w = o2w * w_in;

      // Create a ray, uniformly at random, pointing away from the intersect in the direction of the normal hemisphere
      Ray ray = Ray(hit_p + EPS_F*w, w);
      ray.min_t = EPS_F;

      Intersection lightSource;
      if (bvh->intersect(ray, &lightSource)) {        // if sample ray intersects something
          Vector3D light = lightSource.bsdf->get_emission();    // get the light value
          L_out += light * isect.bsdf->f(hit_p, w_in) * cos_theta(w_in);    // estimate
      }
  }
  return ((2.0 * PI) / num_samples) * L_out;    // normalize and return
}

Vector3D
PathTracer::estimate_direct_lighting_importance(const Ray &r,
                                                const Intersection &isect) {
  // Estimate the lighting from this intersection coming directly from a light.
  // To implement importance sampling, sample only from lights, not uniformly in
  // a hemisphere.

  // make a coordinate system for a hit point
  // with N aligned with the Z direction.
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  // w_out points towards the source of the ray (e.g.,
  // toward the camera if this is a primary ray)
  const Vector3D hit_p = r.o + r.d * isect.t;
  const Vector3D w_out = w2o * (-r.d);
  Vector3D L_out, sum;

  for (SceneLight *light: scene->lights) {
      Vector3D w, w_in;
      double distance, pdf;
      for (int i = 0; i < ns_area_light; i++) {
          Vector3D lightSample = light->sample_L(hit_p, &w, &distance, &pdf);
          w_in = w2o * w;
          if (w_in.z >= 0) {
              Ray shadow = Ray(hit_p, w);
              shadow.min_t = EPS_F;
              shadow.max_t = distance - EPS_F;
              Intersection shadowIsect;
              if (!bvh->intersect(shadow, &shadowIsect)) {
                  if (light->is_delta_light()) {
                      L_out += (lightSample * cos_theta(w_in) * isect.bsdf->f(w_out, w_in)) / pdf;
                      break;
                  } else {
                      sum += (lightSample * cos_theta(w_in) * isect.bsdf->f(w_out, w_in)) / pdf;
                  }
              }
          }
      }
  }
  return L_out + sum / ns_area_light;
}

Vector3D PathTracer::zero_bounce_radiance(const Ray &r,
                                          const Intersection &isect) {
    return isect.bsdf->get_emission();        // return the light directly from the intersection
}

Vector3D PathTracer::one_bounce_radiance(const Ray &r,
                                         const Intersection &isect) {
  // TODO: Part 3, Task 3
  // Returns either the direct illumination by hemisphere or importance sampling
  // depending on `direct_hemisphere_sample`
  return (direct_hemisphere_sample) ?
            estimate_direct_lighting_hemisphere(r, isect) :
            estimate_direct_lighting_importance(r, isect);
}

Vector3D PathTracer::at_least_one_bounce_radiance(const Ray &r,
                                                  const Intersection &isect) {
  Matrix3x3 o2w;
  make_coord_space(o2w, isect.n);
  Matrix3x3 w2o = o2w.T();

  Vector3D hit_p = r.o + r.d * isect.t;
  Vector3D w_out = w2o * (-r.d);

  Vector3D L_out(0, 0, 0);



  return L_out;
}

Vector3D PathTracer::est_radiance_global_illumination(const Ray &r) {
  Intersection isect;
  Vector3D L_out;

  // You will extend this in assignment 3-2.
  // If no intersection occurs, we simply return black.
  // This changes if you implement hemispherical lighting for extra credit.

  // The following line of code returns a debug color depending
  // on whether ray intersection with triangles or spheres has
  // been implemented.
  //
  // REMOVE THIS LINE when you are ready to begin Part 3.

  if (!bvh->intersect(r, &isect))
    return envLight ? envLight->sample_dir(r) : L_out;


  L_out = (isect.t == INF_D) ? debug_shading(r.d) : normal_shading(isect.n);

  return zero_bounce_radiance(r, isect) + one_bounce_radiance(r, isect);
}

void PathTracer::raytrace_pixel(size_t x, size_t y) {
  // TODO (Part 5):
  // Modify your implementation to include adaptive sampling.
  // Use the command line parameters "samplesPerBatch" and "maxTolerance"

  int num_samples = ns_aa;          // total samples to evaluate
  Vector2D origin = Vector2D(x, y); // bottom left corner of the pixel
  Vector3D estRadiance = Vector3D(.0, .0, .0);

  for (int i=0; i<num_samples; i++) {
      // get random pixel sample and normalize by image dimensions
      Vector2D pixelSample = origin + gridSampler->get_sample();
      pixelSample.x /= sampleBuffer.w;
      pixelSample.y /= sampleBuffer.h;
      // generate ray and estimate illumination, update total est radiance
      Ray sampleRay = camera->generate_ray(pixelSample.x, pixelSample.y);
      estRadiance += PathTracer::est_radiance_global_illumination(sampleRay);
  }
  estRadiance /= ns_aa;     // normalize by number of samples

  sampleBuffer.update_pixel(estRadiance, x, y);
  sampleCountBuffer[x + y * sampleBuffer.w] = num_samples;
}

void PathTracer::autofocus(Vector2D loc) {
  Ray r = camera->generate_ray(loc.x / sampleBuffer.w, loc.y / sampleBuffer.h);
  Intersection isect;

  bvh->intersect(r, &isect);

  camera->focalDistance = isect.t;
}

} // namespace CGL
