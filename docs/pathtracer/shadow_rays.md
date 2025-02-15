---
layout: default
title: "(Task 4) Shadow Rays"
permalink: /pathtracer/shadow_rays
---

# (Task 4) Shadow Rays

In this task you will modify `Pathtracer::trace_ray` to implement accurate shadows.

Currently `Pathtracer::trace_ray` computes the following:

* It computes the intersection of ray `r` with the scene.
* It computes the amount of light arriving at the hit point `hit.position` (the irradiance at the hit point) by integrating radiance from all scene light sources.
* It computes the radiance reflected from the hit point in the direction of `-r`. (The amount of reflected light is based on the BSDF of the surface at the hit point.)

Shadows occur when another scene object blocks light emitted from scene light sources towards the hit point. Fortunately, determining whether or not a ray of light from a light source to the hit point is occluded by another object is easy given a working ray tracer (which you have at this point!). **You simply want to know whether a ray originating from the hit point (`hit.position`), and traveling towards the light source (direction to light) hits any scene geometry before reaching the light.**

Your job is to implement the logic needed to compute whether hit point is in shadow with respect to the current light source sample. Below are a few notes:

* Get yourself oriented in the starter code by taking a look at `Pathtracer::trace_ray` in `src/student/pathtracer.cpp`.  Notice that the code, after finding a hit, loops over all light source accumulating radiance. (look for the comment `// loop over all the lights and accumulate radiance`).  For each light the lambda function `sample_light()` is invoked, and in that function the key line is the line that adds radiance into the variable `radiance_out`.  __This code is the code where the refection equation is being evaluated.__

* In the starter code, notice that when the code calls `light.sample(hit.position)`, it returns the caller a `Light_sample sample`. (You might want to take a look at `rays/light.h` for the definition of `struct Light_sample` and `class light`.) A `Light_sample` contains the fields `radiance`, `pdf`, `direction`, and `distance`. In particular, `sample.direction` is the direction from the surface hit point to the point on the light source being sampled, and `sample.distance` is the distance from the hit point to the light sample. Given this information, all you need to do is determine if there is any scene geometry _closer than_ that sample point.

* A common ray tracing pitfall is for the "shadow ray" shot into the scene accidentally hits the same object as `r` did. (The surface is erroneously determined to be occluded because the shadow ray is determined to hit the surface itself!). We recommend that you make sure the origin of the shadow ray is offset from the surface to avoid these erroneous "self-intersections". For example, consider setting the ray's `dist_bounds` appropriately to avoid hits with very small `t` values, or adjust the origin of the shadow ray to be `hit.position + epsilon * sample.direction` instead of simply `hit.position`. `EPS_F` is defined in for this purpose (see `lib/mathlib.h`).

* Another common pitfall is forgetting that the surface is not in shadow if the shadow ray hits scene geometry _after reaching the light_. (Also note that `Ray` has a member called `dist_bound`...)
* You will find it useful to debug your shadow code using the `DirectionalLight` since it produces hard shadows that are easy to reason about.
* When you get started on Task 4, comment out the line `Spectrum radiance_out = Spectrum(0.25f);` and initialize the `radiance_out` to be the correct value. Hint: is there supposed to have any light before we even start considering each light sample?

At this point you should be able to render very striking images with shadows. 

## Sample results:

At this point, you can add all kinds of lights using "New Light" in Layout mode, except for Sphere Light and Environment Map which you have the option to implement later (Note that you can still fill in `Sphere::Uniform::sample` in `Samplers.cpp` now to view the result of a mesh under Sphere Light).  Here are some examples:

A sphere and a cube with hemishphere lighting

![shadow_hemisphere](new_results/cube_sphere_hemisphere.png)

Hex and cube under diretional lighting

![ref1](new_results/ref1.png)

Bunny on a plane under point light

![ref1](new_results/ref2.png)

Spot on a sphere under diretional lighting

![ref1](new_results/ref3.png)


Spot on a sphere under hemisphere lighting

![ref1](new_results/ref4.png)


