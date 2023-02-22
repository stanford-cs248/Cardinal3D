---
layout: default
title: "(Task 2) Intersecting Objects"
permalink: /pathtracer/intersecting_objects
---

# (Task 2) Intersecting Objects

Now that your ray tracer generates camera rays, we need to be able to answer the core query in ray tracing: "does this ray hit this object?" In this task, you will start by implementing ray-object intersection routines against the two types of objects in the starter code: __triangles and spheres__ (spheres are extra credit). Later, we will use a BVH to accelerate these queries, but for now we consider an intersection test against a single object.

First, take a look at the `rays/object.h` for the interface of `Object` class. An `Object` can be **either** a `Tri_Mesh`, a `Shape`, a BVH (which you will implement in Task 3), or a list of `Objects`. Right now, we are only dealing with the `Tri_Mesh` and `Shape` cases. You can find their interfaces in `rays/tri_mesh.h` and `rays/shapes.h`, respectively. `Tri_Mesh` contains a BVH of `Triangle` primitives, and so in this task you will be working with the `Triangle` class. For `Shape`, you are going to work with `Sphere`s, one major type of `Shape` in Cardinal 3D. 

You must implement the `hit` routine for both `Triangle` and `Sphere`. `hit` takes in a ray, and returns a `Trace` structure, which contains information on whether the ray hits the object and, if it does hit, the information describing the surface at the point of the hit. See `rays/trace.h` for the definition of `Trace`.

In order to correctly implement `hit` you need to understand some of the fields in the Ray structure defined in `lib/ray.h`.

* `point`: represents the 3D point of origin of the ray
* `dir`: represents the 3D direction of the ray (this direction will be normalized)
* `dist_bounds`: correspond to the minimum and maximum points on the ray with its x-component as the lower bound and y-component as the upper bound. That is, intersections that lie outside the [`ray.dist_bounds.x`, `ray.dist_bounds.y`]  range __should not be considered valid intersections with the primitive__.

One important detail of the `Ray` structure is that `dist_bounds` is a mutable field of the ray. This means that this field can be modified by constant member functions such as `Triangle::hit`. When finding the first intersection of a ray and the scene, you almost certainly want to update the ray's `dist_bounds` value after finding each hit with scene geometry. _If you find a ray-triangle hit at distance 't', what should you do to the ray's tiem bounds so that all future intersection tests are aware of this prior hit?_. 

By bounding the ray as tightly as possible, your ray tracer will be able to avoid unnecessary tests with scene geometry that is known to not be able to result in a closest hit, resulting in higher performance.

---

### **Step 1: Intersecting Triangles**

The first intersect routine that the `hit` routines for the triangle mesh in `student/tri_mesh.cpp`.

You may implement ray-triangle intersection however you wish, but we recommend you implement ray-triangle intersection using the method described in the [lecture slides](https://gfxcourses.stanford.edu/cs248a/winter23/lecture/geometricqueries/slide_23). Further details of implementing this method efficiently are given in [these notes](ray_triangle_intersection.md).

There are two important details you should be aware of about intersection:

* When finding the first-hit intersection with a triangle, you need to fill in the `Trace` structure with details of the hit. The structure should be initialized with:
    
    * `hit`: a boolean representing if there is a hit or not
    * `distance`: the ray's _t_-value of the hit point
    * `position`: the exact position of the hit point. This can be easily computed from the `distance` of the hit, and ray's `point` and `dir`.
    * `normal`: the normal of the surface at the hit point. This normal should be the interpolated normal (obtained via interpolation of the per-vertex normals according to the barycentric coordinates of the hit point)

Once you've successfully implemented triangle intersection, you will be able to render many of the scenes in the `/media` directory. However, your ray tracer will be very, very slow on high triangle count scenes.

While you are working with `student/tri_mesh.cpp`, you should implement `Triangle::bbox` as well, which is important for Task 3.

Tip: [Visualization of normals](visualization_of_normals.md) might be very helpful with debugging.

### **Step 2: Intersecting Spheres (EXTRA CREDIT ONLY)**

As you did with triangles, implement the `hit` routines for the `Sphere` class in `student/shapes.cpp`. Remember that your intersection tests should respect the ray's `dist_bound`.

