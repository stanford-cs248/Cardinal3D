# Stanford CS248A Assignment 3 (PathTracer)

We have created a [Wiki](https://stanford-cs248.github.io/Cardinal3D/) that will be the primary source of information about this assignment. (This is the same wiki as assignment 2.) On that page in the section called ["Pathtracer"](https://stanford-cs248.github.io/Cardinal3D/pathtracer/) where you will find all the details about how what you need to implement, and tips/guidelines for implementing it.  The [render mode](https://stanford-cs248.github.io/Cardinal3D/guide/render/) section of the user guide documents how to use the renderer. This document only contains administrative details about building the starter code, grading, and submission.

## Due Date

The assignment is due Feb. 20 at 11:59 PM.

## Submission Instructions
Please zip the following files and upload your zipped file to Gradescope.

* `Cardinal3D/src` folder
* writeup (.txt, .doc, .pdf). Your writeup should document any extra credt tasks you did, and provide instructions for how the grader should confirm correctness. e.g., render this scene and you should see an image that looks like this...

One team only needs one submission, please tag your teammates in your submission.

## Code Environment

This codebase should compile on Linux, Mac OS X, and Windows on a typical environment. Check out the [building Guide](https://stanford-cs248.github.io/Cardinal3D/build/) to install dependencies and building the code. 

When you have successfully built your code, you should get an executable named `Cardinal3D`. Upon starting the program, it should be in model mode. The [User Guide](https://stanford-cs248.github.io/Cardinal3D/guide/) details how to use the interface. 

### Starting off with your codebase from PA2

The MeshEdit and PathTracer assignments use the same source code for Cardinal3D. You may do any one of the following to get started on PathTracer:
1. Rebase your MeshEdit repo to The Cardinal3D remote repository to receive all the new PA3-specific starter code changes.
2. You may make a fresh clone of this repository, and then copy over your PA2 solution from `meshedit.cpp` to your new local repository. (if you wish to use your assignment 2 solution to make models that you can ray trace in this assignment.) You do not need a correctly functioning MeshEdit solution to work on PathTracer, but you may appreciate having more Cardinal3D functionality at your disposal.

### Evaluation and Grading ###

In this assignment you will perform a subset of the tasks described in the [Pathtracer section](https://stanford-cs248.github.io/Cardinal3D/pathtracer/) of the wiki.

You are required to complete the following tasks to receive the baseline score of 92 points.

* Task 1: Generating Camera Rays (15 pts)
* Task 2: Ray-Triangle Intersection (22 pts)
* Task 3: BVH intersection (30 pts)
* Task 4: Shadow Rays (10 pts)
* Task 6: Perfect Mirror Reflection (15 pts) 

For additional credit you can attempt additional tasks: We'll cap all scores at 104.

* Ray-Sphere intersection (1 pt, described in Task 2)
* Task 5: Global illumination path tracing with Russian roulette (6 pts)
* Transmissive surfaces, w/ transmission given by the Fresnel equations) (4 pts, described in Task 6)

Additional ideas: (credit will be on a case-by-case basis).  Note that good final projects might involve extending your path tracer with these more advanced features.

* Evaluate the tradeoffs in terms of construction cost vs. tracing efficiency of various BVH build algorithms.  Better builds will reduce ray tracing cost, but may be more costly to construct the BVH.  You could try [fast, simple builds](https://luebke.us/publications/eg09.pdf) or [state-of-the-art build algorithms](https://research.nvidia.com/sites/default/files/pubs/2013-07_Fast-Parallel-Construction/karras2013hpg_paper.pdf)
* Importance sampled environment lighting. Ask us for instructions.
* Better sampling algorithms
* More advanced materials and new BSDFs (such as a subsurface scattering material)
* More advanced camera simulation (a camera with an aperture)
* Motion blur
* Ray trace more complex primitives like subdivision surfaces.
* Volume rendering.
* More BVH traversal performance. The start code parallelizes ray tracing over threads, but students that have knowledge of SIMD execution may wish to use tools like [ISPC](https://ispc.github.io/) to implement a vectorized traversal for even higher performance.


## Acknowledgement

CS248A course staff would like to thank Professor Keenan Crane and his course assistants for the initial development of assignment materials.
