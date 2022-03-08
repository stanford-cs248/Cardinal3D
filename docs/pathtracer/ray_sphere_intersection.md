---
layout: default
title: Ray Sphere Intersection
permalink: /pathtracer/ray_sphere_intersection
grand_parent: "A3: Pathtracer"
parent: (Task 2) Intersections
---

# Ray Sphere Intersection

## Step 2: `Sphere::hit`

You also need to implement the `hit` routines for the `Sphere` class in `student/shapes.cpp`. Remember that your intersection tests should respect the ray's `dist_bounds`, and that normals should be out-facing.

<center><img src="figures\sphere_intersect_diagram.png" style="height:320px"></center>
<center><img src="figures\sphere_intersect_eqns.png" style="height:400px"></center>

**Tip 1:** take care **NOT** to use the `Vec3::normalize()` method when computing your
normal vector. You should instead use `Vec3::unit()`, since `Vec3::normalize()`
will actually change the `Vec3` calling object rather than returning a
normalized version.


**Tip 2:** A common mistake is to forget to check the case where the first
interesection time t1 is out of bounds but the second interesection time t2 is
(in which case you should return t2).

<center><img src="figures\sphere_specialcase_diagram.png" style="height:220px"></center>
