---
layout: default
title: "Cardinal3D"
permalink: /
---

# Cardinal3D

Welcome to Cardinal3D! This 3D graphics software package includes components for interactive mesh
editing and realistic path tracin. Implementing functionality in these areas
constitutes coursework for CS248A (Computer Graphics: Rendering, Geometry, and Imange Manipulation) at Stanford University

These pages describe how to set up with Cardinal3D. Start here!
- [Git Setup](git): create a private git mirror that can pull changes from Cardinal3D (optional). 
- [Building Cardinal3D](build): build and run Cardinal3D on various platforms.

__How to use Cardinal3D:__ read here to understand how to use the software. (But not all functionality will work as described until you implement it!)
- [User Guide](guide): learn the intended functionality for end users.

__How to implement Cardinal3D:__ The developer manual describes what you must implement to complete Cardinal3D. It is organized in terms of the main components of the software. Note that only MeshEdit and PathTracer are given to students in CS248A as assignments, but feel free to check out the Animation section to have more fun with Cardinal3D.
- [MeshEdit](meshedit)
- [PathTracer](pathtracer)
- [Animation](animation)

## Project Philosophy

Welcome to your first day of work at Cardinal Industries! 
You are tasked to implement core features in Cardinal Industries' flagship product
Cardinal3D, which is a modern package for 3D modeling and rendering.
In terms of basic structure, this package doesn't look much different from
"real" 3D tools like Maya, Blender, modo, or Houdini. Your overarching goal is
to use the developer manual to implement a package that
works as described in the [User Guide](guide), much as you would at a real
software company (more details below).

Note that the User Guide is **not** an Assignment Writeup. The User Guide
contains only instructions on how to use the software, and serves as a
high-level specification of _what the software should do_. The Developer Guide
contains information about the internals of the code, i.e., _how the software
works_. This division is quite common in software development: there is a
**design specification** or "design spec", and an **implementation** that
implements that spec. Also, as in the real world, the design spec does _not_
necessarily specify every tiny detail of how the software should behave! Some
behaviors may be undefined, and some of these details are left up to the party
who implements the specification. A good example you have already seen is
OpenGL, which defines some important rules about how rasterization should
behave, but is not a "pixel-exact" specification. In other words, two different
OpenGL implementations from two different vendors (Intel and NVIDIA, say) may
produce images that differ by a number of pixels. Likewise, in this assignment,
your implementation may differ from the implementation of your classmates in
terms of the exact values it produces, or the particular collection of
corner-cases it handles. However, as a developer you should strive to provide a
solution that meets a few fundamental criteria:

*   [Failing gracefully](https://en.wikipedia.org/wiki/Fault_tolerance) is
preferable to failing utterly---for instance, if a rare corner case is difficult
to handle, it is far better to simply refuse to perform the operation than to
let the program crash! 
*   Your implementation should follow the [principle of least
surprise](https://en.wikipedia.org/wiki/Principle_of_least_astonishment). A user
should be able to expect that things behave more or less as they are described
in the User Guide. 
*   Finally, you should take pride in your craft. Beautiful things just tend to work better.

Just to reiterate the main point above:

**As in real-world software development, we will not specify every little detail
about how methods in this assignment should work!**

If you encounter a tough corner case (e.g., "how should edge flip behave for a
tetrahedron"), we want you to _think about what a good **design choice** might
be_, and implement it to the best of your ability. This activity is part of
becoming a world-class developer. However, we are more than happy to discuss
good design choices with you, and you should also feel free to discuss these
choices with your classmates. Practically speaking, it is ok for routines to
simply show an error if they encounter a rare and difficult corner case---as long as it
does not interfere with successful operation of the program (i.e., if it does
not crash or yield bizarre behavior). Your main goal here above all else should be 
to develop _effective tool for modeling, rendering, and animation_.
