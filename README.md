# Stanford CS248A Assignment 2 (Mesh Editing)

We have created a [Wiki](https://stanford-cs248.github.io/Cardinal3D/) that will be the primary source of information about this assignment. On that page you will find all the details about what you need to implement, etc. This document only contains administrative details about building the starter code, grading, and submission.

## Due Date

The assignment is due Feb. 9 at 11:59 PM.

## Submission Instructions
Please zip the following files and upload your zipped file to Gradescope.

* `Cardinal3D/src` folder
* `model.dae`
* writeup (.txt, .doc, .pdf)

One team only needs one submission, please tag your teammates in your submission.

## Code Environment

This codebase should compile on Linux, Mac OS X, and Windows on a typical environment. Check out the [building Guide](https://stanford-cs248.github.io/Cardinal3D/build/) to install dependencies and building the code. 

When you have successfully built your code, you should get an executable named `Cardinal3D`. Upon starting the program, it should be in model mode. The [User Guide](https://stanford-cs248.github.io/Cardinal3D/guide/) details how to use the interface. 

### Usage Notes
* Any mesh with .dae can be imported from media folder, others can be created through "New Object".

* Editing the sphere mesh: click on the sphere -> click "Edit Mesh" -> Dropdown select "None" -> click Smaller "Edit Mesh".


## Evaluation
For this assignment, you will implement methods in `meshEdit.cpp`.

The User Guide on the wiki describes a large number of features that are in principle available in MeshEdit mode. You have to successfully implement a subset of the features as __basic tasks__, implementing additional features beyond the basic tasks will earn you extra points.

The basic tasks, and the percentage of the grade (92 pts total) they correspond to, are:

* Four local operations: **EdgeCollapse**, **EdgeFlip**, **EdgeSplit** and **FaceBevel** (8 pts each)
* Four global operations: **Triangulation**, **LinearSubdivision** and **CatmullClarkSubdivision** (10 pts each) and **Simplification** (20 pts)
* Create one beautiful 3D model using Cardinal3D (10 pts)

In other words, everyone has to implement **EdgeCollapse**, **EdgeFlip**, **EdgeSplit**, **FaceBevel**, **Triangulation**, **LinearSubdivision**, **Catmull-Clark** and **Simplification**. These features are the bare minimum needed to model interesting subdivision surfaces; **Triangulation** is necessary in order to do the global remeshing task(s). Note that some of the global tasks require that you implement specific local operations! For instance, the implementation of **Simplification** depends on **EdgeCollapse**. In summary, we list all the local operations as follows (these operations are described in the User Guide):

* **VertexBevel**
* **EdgeBevel**
* **FaceBevel** - basic task
* **EraseVertex**
* **EraseEdge**
* **EdgeCollapse** - basic task
* **FaceCollapse**
* **EdgeFlip** - basic task
* **EdgeSplit** - basic task

The global operations, and their dependency on local operations, are as follows:

* **Triangulation** - basic task
* **LinearSubdivision** - basic task
* **CatmullClarkSubdivision** - basic task
* **Simplification** - basic task
* **LoopSubdivision** - depends on **EdgeSplit** and **EdgeFlip**
* **IsotropicRemeshing** - depends on **EdgeSplit**, **EdgeFlip**, and **EdgeCollapse**

Each additional local operation beyond the basic tasks will be worth 1 pts; each additional global operation will be worth 2 pts. The maximum possible grade on the assignment is 100 pts.

## Grading

The minimal set of test cases we will evaluate on are:

* Edge flip: sphere and dodecahedron.dae (flip must work for non-triangle faces)
* Edge split: sphere and square (boundary)
* Edge collapse: sphere and square (boundary)
* Bevel vertex/edge/face: cube.dae
* Erase vertex/edge: sphere
* Face collapse: sphere
* Triangulation: dodecahedron.dae
* Linear Subdivision: sphere
* Catmull-Clark Subdivision: sphere
* Simplification: sphere, teapot.dae, cow.dae, dragon2.dae
* Loop Subdivision: sphere
* IsotropicRemeshing: cow.dae, peter.dae

Please note that your code must be robust enough to handle other test cases as well.

#### America's Next Top 3D Model

Every student is required to submit a 3D model created from cube.dae (or other primitives) using their implementation of Cardinal3D, which will be automatically entered into a class-wide 3D modeling competition. Models will be critiqued and evaluated based on both technical sophistication and aesthetic beauty. Note: Use of any other 3D package (e.g., free or commercial 3D modelers like Maya or Blender) is strictly prohibited! This model must be created by opening cube.dae, applying the operations implemented as part of the assignment, and saving the result.

NOTE: We are expecting some high-quality output here---or at least some creativity! Don't just brush this one off. :-)

Include this model in the root directory of your submission as model.dae.

## Writeup

Additionally, we want you to submit a short document explaining what you have implemented, and any particular details of your submission. If your submission includes any implementations which are not entirely functional, please detail what works and what doesn't, along with where you got stuck. This document does not need to be long; correctly implemented features may simply be listed, and incomplete features should be described in a few sentences at most.

The writeup can be a pdf, markdown, or plaintext file. Include it in the root directory of your submission as writeup.pdf, writeup.md, or writeup.txt.

If we cannot find this writeup in your submission, no addtional implemented operations will be graded.

## Acknowledgement

CS248A course staff would like to thank Professor Keenan Crane and his course assistants for the initial development of assignment materials.
