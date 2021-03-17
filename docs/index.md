<style>
          body {
    padding: 100px;
    width: 1000px;
    margin: auto;
    text-align: left;
    font-weight: 300;
    font-family: 'Open Sans', sans-serif;
    color: #121212;
  }
  h1, h2, h3, h4 {
    font-family: 'Source Sans Pro', sans-serif;
  }
</style>

# <center> **CS 184: Computer Graphics and Imaging, Spring 2021** <!-- omit in toc -->

## <center> **Project 3: Path Tracing** <!-- omit in toc -->

## <center> **Frank Warren** </center> <!-- omit in toc -->

## **Contents** <!-- omit in toc -->

- [**Overview**](#overview)
- [**Part 1: Ray Generation and Scene Intersection**](#part-1-ray-generation-and-scene-intersection)
- [**Part 2: Bounding Volume Hierarchy**](#part-2-bounding-volume-hierarchy)
- [**Part 3: Direct Illumination**](#part-3-direct-illumination)
- [**Part 4: Global Illumination**](#part-4-global-illumination)
- [**Part 5: Adaptive Sampling**](#part-5-adaptive-sampling)
- [**EXAMPLE CODE BLOCK**](#example-code-block)

## **Overview**

The aim of this project is to implement a physics-based graphics renderer using a light pathtracing algorithm. Some of the ideas that will be used in this project include ray-scene intersection, acceleration structures, and physically based lighting and materials. The completed project will be able to generate realistic pictures.

## **Part 1: Ray Generation and Scene Intersection**

When we want to render something, it is given as a normalized `(x, y)` coordinate in the **image space**. Basically, this is just some pixel. Then, given the vertical and horizonal field of view angles `hFov` and `vFov`, we can translate the location of this pixel into the 3d **camera space**. This space is defined with a camera at the origin looking down the $-Z$ axis toward a plane (the **virtual sensor**) that's perpendicular to the $Z$ axis. The top right and bottom left of this plane (from the camera's point of view) is at the points $(tan(0.5 \cdot hFov), tan(0.5 \cdot vFov), -1)$ and $(-tan(0.5 \cdot hFov), -tan(0.5 \cdot vFov), -1)$ respectively. Finally, we can translate the point in the camera space into a point in the **world space** by using the rotation matrix `c2w`. The world space is pretty much the same idea as the camera space but now the camera isn't locked to the origin facing down the $-Z$ axis.  

After all of this is done, we are left with a point in the world space which we can use, along with the position of the camera, to generate a ray pointed at the point in world space from the camera.  

Now that we've implemented a way to generate rays, we want to determine if those rays intersect objects in a scene. To determine if a ray intersects some triangle, we can rely on the plane equation $p:(p-p') * N = 0$. The idea of determining if a point is on a plane by examining if it forms a line perpendicular to the normal of the plane with some known point the plane carries over exactly to finding intersections with triangles. Triangles are linear, so to figure out if a ray intersects a triangle, we use the same strategy but check to see if the resulting intersection point is inside the triangle (by comparing normals).  

There is an optimized algorithm for finding the time a ray intersects a triangle and the barycentric coordinates of the intersection known as the [Moller Trumbore Algorithm](https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm). I implemented this algorithm in my implementation for triangle intersections.

| <img src="./images/mollertrumbore.jpg" width=550> |
|:--:|
|Moller Trumbore Algorithm|

Once a time and barycentric coordinate pair is found, we must verify that the given time is nonnegative, the barycentric coordinates sum to 1, and the intersection is in the triangle.  

The process for sphere intersection is pretty much the same as triangle intersection. We take advantage of the general sphere defining formula to solve for $t$ values. There was no special algorithm used to implement sphere-ray intersection. Finding the time(s) a ray intersects a sphere basically boiled down to solving the quadratic formula.  

Here's some images with normal shading generated using these ray generation and scene intersection implementations:  

| <img src="./images/spheres.png">| <img src="./images/banana.png"> | <img src="./images/bunny.png"> |
|:--:|:--:|:--:|
|  | | |

## **Part 2: Bounding Volume Hierarchy**

A [bounding volume hierarchy](https://en.wikipedia.org/wiki/Bounding_volume_hierarchy) (BVH) is a tree structure used to organized a set of geometric objects (things with length, volume, etc.). The basic idea of a bounding volume is to define boxes that contain objects in space such that we can efficiently find them. This is useful in ray tracing because everytime we cast a ray into a scene, we need to find out exactly what objects it will intersect, and with a BVH we can easily narrow down the objects by simply ignoring the ones we know *for sure* won't be intersected. In this project, the BVH is implemented as a binary tree. Each node points to a bounding box that contains a set of primitives (geometric objects). The left and right children of each nonleaf node points to nodes that represent the two partitions of the bounding box. The primitives in each node are tracked with two iterators: `start` and `end`.  

The bulk of the work when constructing a BVH comes from choosing and implementing a heuristic to split the objects in a scene in a way that minimizes the costs associated with ray tracing computations (i.e. the scene is split not too broadly or finely, along well chosen split points, etc). There are a number of metrics that can be considered in a heuristic such as the surface area of each primitive, the average location of each primitive, the number of primitives that lay on each side of a potential split plane, and others.  

In this project, the heuristic that was used to split a bounding box started by finding the axis for the box's largest dimension. Then, the split point along that dimesnion was computed as the average centroid (in that dimension) of the primitive's bounding boxes. After both of these steps, we essential have direction and a point and therefore we have a split plane.  

I actually spent an embarassingly long amount of time on this part because after implemented a BVH correctly and the methods for intersecting lines with bounding volumes, I forgot to change the provided code to loop ***only*** over primitives contained inside a bounding box that's intersected. Every time a ray intersected with a bounding box, I was testing intersection with every single primitive in the scene. Also, I was having a problem where my images would render flipped with some small amount of seemingly random primitives not being rendered. This was due to a short circuit in the expression `intersect(ray, intersect, left) || intersect(ray, intersect, right)` that was causing the right side to not be evaluated everytime there happened to be an intersect in the left bounding box, even though the right bounding box might contain an object that would've been intersected first.  

Anyways, here's some complex geometries that render much more efficiently after BVH was implemented.  

| <img src="./images/dragon.png">| <img src="./images/cblucy.png"> | <img src="./images/wall-e.png"> |
|:--:|:--:|:--:|
| Rendered in 0.0495s |  Rendered in 0.0619s | Rendered in 0.0753s |

Before implementing BVH, these scenes took at least half a minute each to render. I've never felt so powerful.

## **Part 3: Direct Illumination**

TODO:

- Walk through both implementations of the direct lighting function.
- Show some images rendered with both implementations of the direct lighting function.
- Focus on one particular scene with at least one area light and compare the noise levels in soft shadows when rendering with 1, 4, 16, and 64 light rays (the -l flag) and with 1 sample per pixel (the -s flag) using light sampling, not uniform hemisphere sampling.
- Compare the results between uniform hemisphere sampling and lighting sampling in a one-paragraph analysis.

## **Part 4: Global Illumination**

TODO:

- Walk through your implementation of the indirect lighting function.
- Show some images rendered with global (direct and indirect) illumination. Use 1024 samples per pixel.
- Pick one scene and compare rendered views first with only direct illumination, then only indirect illumination. - Use 1024 samples per pixel. (You will have to edit PathTracer::at_least_one_bounce_radiance(...) in your code to generate these views.)
- For CBbunny.dae, compare rendered views with max_ray_depth set to 0, 1, 2, 3, and 100 (the -m flag). Use 1024 samples per pixel.
- Pick one scene and compare rendered views with various sample-per-pixel rates, including at least 1, 2, 4, 8, 16, 64, and 1024. Use 4 light rays.
- You will probably want to use the instructional machines for the above renders in order to not burn up your own computer for hours.

## **Part 5: Adaptive Sampling**

TODO:

- Walk through your implementation of the adaptive sampling.
- Pick one scene and render it with at least 2048 samples per pixel. Show a good sampling rate image with clearly visible differences in sampling rate over various regions and pixels. Include both your sample rate image, which shows your how your adaptive sampling changes depending on which part of the image you are rendering, and your noise-free rendered result. Use 1 sample per light and at least 5 for max ray depth.

## **EXAMPLE CODE BLOCK**

```C++
Halfedge CIter h = v->halfedge();
do {
  // Process h->face() in this loop
  h = h->twin()->next();
} while (h != v->halfedge());
```