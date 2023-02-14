---
layout: default
title: "PathTracer Overview"
permalink: /pathtracer/
---

# PathTracer Overview

PathTracer is (as the name suggests) a simple path tracer that can render scenes with global illumination. The requires part of CS248A will ask you to perform an efficient implementation of **ray-scene geometry queries**, very basic materials, and shadowed direct lighting. For additional credit, or for your final project if desired, you can **add full path tracing ability to simulate how light bounces around the scene**. Path tracing will allow your renderer to synthesize much higher-quality images. Much like in MeshEdit, input scenes are defined in COLLADA files, so you can create your own scenes to render using Cardinal3D or other free software like [Blender](https://www.blender.org/).

![CBsphere](new_results/32k_large.png)

Implementing the functionality of PathTracer is split into a number of tasks, and here are the instructions for each of them:
- [(Task 1) Generating Camera Rays](camera_rays.md)
- [(Task 2) Intersecting Objects](intersecting_objects.md)
- [(Task 3) Bounding Volume Hierarchy](bounding_volume_hierarchy.md)
- [(Task 4) Shadow Rays](shadow_rays.md)
- [(Task 5) Path Tracing](path_tracing.md).
- [(Task 6) Materials](materials.md)

<!-- - [(Task 7) Environment Lighting](environment_lighting.md) -->

The files that you will work with for PathTracer are all under `src/student` directory. Some of the particularly important ones are outlined below. Methods that we expect you to implement are marked with "TODO (PathTracer)", which you may search for.

You are also provided with some very useful debugging tool in `src/student/debug.h` and `src/student/debug.cpp`. Please read the comments in those two files to learn how to use them effectively.

| File(s)  |      Purpose      |  Need to modify? |
|----------|-------------------|------------------|
| `student/pathtracer.cpp` |  This is the main work horse class. Everything begins here.  Inside the `Pathtracer` class everything begins with the method `Pathtracer::trace_pixel` in pathtracer.cpp. This method computes the value incoming radiance along a ray through the specified pixel in the output image. | Yes |
| `student/camera.cpp` | This is the code that generates world-space camera rays that are traced into the scene. You will need to modify `Camera::generate_ray` in Part 1 of the assignment to generate camera rays. |  Yes |
| `student/tri_mesh.cpp`, `student/shapes.cpp` | Scene objects (e.g., triangles and spheres) are instances of the `Object` class interface defined in `rays/object.h`. You will need to implement the `bbox` and intersect routine `hit` for spheres.  We give you the implementation for triangles. |   Yes |
|`student/bvh.inl`|A major portion of the assignment concerns implementing a bounding volume hierarchy (BVH) that accelerates ray-scene intersection queries. Note that a BVH is also an instance of the Object interface (A BVH is a scene object that itself contains other primitives.)|Yes|
|`rays/light.h`|Describes lights in the scene. The initial starter code has working implementations multiple types of light sources.|No|
|`lib/spectrum.h`|Light energy is represented by instances of the Spectrum class. While it's tempting, we encourage you to avoid thinking of spectrums as colors -- think of them as a measurement of energy over many wavelengths. Although our current implementation only represents spectrums by red, green, and blue components (much like the RGB representations of color you've used previously in this class), this abstraction makes it possible to consider other implementations of spectrum in the future. Spectrums can be converted into a vector using the `Spectrum::to_vec` method.| No|
|`student/bsdf.cpp`|Contains implementations of several BSDFs (diffuse, mirror, glass). For each, you will define the distribution of the BSDF and write a method to sample from that distribution.|Yes|
|`student/samplers.cpp`|When implementing raytracing and environment lights, we often want to sample randomly from a hemisphere, uniform grid, or shphere. This file contains various functions that simulate such random sampling.|Yes|


