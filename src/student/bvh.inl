
#include "../rays/bvh.h"
#include "debug.h"
#include <stack>

namespace PT {

// construct BVH hierarchy given a vector of prims
template<typename Primitive>
void BVH<Primitive>::build(std::vector<Primitive>&& prims, size_t max_leaf_size) {

    // NOTE (PathTracer):
    // This BVH is parameterized on the type of the primitive it contains. This allows
    // us to build a BVH over any type that defines a certain interface. Specifically,
    // we use this to both build a BVH over triangles within each Tri_Mesh, and over
    // a variety of Objects (which might be Tri_Meshes, Spheres, etc.) in Pathtracer.
    //
    // The Primitive interface must implement these two functions:
    //      BBox bbox() const;
    //      Trace hit(const Ray& ray) const;
    // Hence, you may call bbox() and hit() on any value of type Primitive.

    // Keep these two lines of code in your solution. They clear the list of nodes and
    // initialize member variable 'primitives' as a vector of the scene prims
    nodes.clear();
    primitives = std::move(prims);

    // TODO (PathTracer): Task 3
    // Modify the code ahead to construct a BVH from the given vector of primitives and maximum leaf
    // size configuration.
    //
    // Please use the SAH as described in class.  We recomment the binned build from lecture.
    // In general, here is a rough sketch:
    //
    //  For each axis X,Y,Z:
    //     Try possible splits along axis, evaluate SAH for each
    //  Take minimum cost across all axes.
    //  Partition primitives into a left and right child group
    //  Compute left and right child bboxes
    //  Make the left and right child nodes.
    //
    //
    // While a BVH is conceptually a tree structure, the BVH class uses a single vector (nodes)
    // to store all the nodes. Therefore, BVH nodes don't contain pointers to child nodes,
    // but rather the indices of the
    // child nodes in this array. Hence, to get the child of a node, you have to
    // look up the child index in this vector (e.g. nodes[node.l]). Similarly,
    // to create a new node, don't allocate one yourself - use BVH::new_node, which
    // returns the index of a newly added node.
    //
    // As an example of how to make nodes, the starter code below builds a BVH with a
    // root node that encloses all the primitives and its two descendants at Level 2.
    // For now, the split is hardcoded such that the first primitive is put in the left
    // child of the root, and all the other primitives are in the right child.
    // There are no further descendants.

    // edge case
    if(primitives.empty()) {
        return;
    }

    // compute bounding box for all primitives
    BBox bb;
    for(size_t i = 0; i < primitives.size(); ++i) {
        bb.enclose(primitives[i].bbox());
    }

    // set up root node (root BVH). Notice that it contains all primitives.
    size_t root_node_addr = new_node();
    Node& node = nodes[root_node_addr];
    node.bbox = bb;
    node.start = 0;
    node.size = primitives.size();

    // Create bounding boxes for children
    BBox split_leftBox;
    BBox split_rightBox;

    // compute bbox for left child
    Primitive& p = primitives[0];
    BBox pbb = p.bbox();
    split_leftBox.enclose(pbb);

    // compute bbox for right child
    for(size_t i = 1; i < primitives.size(); ++i) {
        Primitive& p = primitives[i];
        BBox pbb = p.bbox();
        split_rightBox.enclose(pbb);
    }

    // Note that by construction in this simple example, the primitives are
    // contiguous as required. But in the students real code, students are
    // responsible for reorganizing the primitives in the primitives array so that
    // after a SAH split is computed, the chidren refer to contiguous ranges of primitives.

    size_t startl = 0;                          // starting prim index of left child
    size_t rangel = 1;                          // number of prims in left child
    size_t startr = startl + rangel;            // starting prim index of right child
    size_t ranger = primitives.size() - rangel; // number of prims in right child

    // create child nodes
    size_t node_addr_l = new_node();
    size_t node_addr_r = new_node();
    nodes[root_node_addr].l = node_addr_l;
    nodes[root_node_addr].r = node_addr_r;

    nodes[node_addr_l].bbox = split_leftBox;
    nodes[node_addr_l].start = startl;
    nodes[node_addr_l].size = rangel;

    nodes[node_addr_r].bbox = split_rightBox;
    nodes[node_addr_r].start = startr;
    nodes[node_addr_r].size = ranger;
}

template<typename Primitive>
Trace BVH<Primitive>::hit(const Ray& ray) const {

    // TODO (PathTracer): Task 3
    // Implement ray - BVH intersection test. A ray intersects
    // with a BVH aggregate if and only if it intersects a primitive in
    // the BVH that is not an aggregate.

    // The starter code simply iterates through all the primitives.
    // Again, remember you can use hit() on any Primitive value.
    Node node = nodes[0];
    BBox node_bbox = node.bbox;
    Vec2 hit_times;
    Trace ret;
    if(node_bbox.hit(ray, hit_times)) {
        hit_helper(ray, node, &ret);
    }
    return ret;
}

template<typename Primitive>
void BVH<Primitive>::hit_helper(const Ray& ray, const Node& node, Trace* ret) const {

    if(node->is_leaf()) {
        for(int i = start; i < start + size; i++) {
            Primitive& p = primitives[i];
            Trace hit = prim.hit(ray);
            *ret = Trace::min(*ret, hit);
        }
    } else {
        Node left_node = nodes[node.l];
        BBox left_bbox = left_node.bbox;
        Vec2 left_hit_times;
        bool left_hit = left_bbox.hit(ray, left_hit_times);

        Node right_node = nodes[node.r];
        BBox right_bbox = right_node.bbox;
        Vec2 right_hit_times;
        bool right_hit = right_bbox.hit(ray, right_hit_times);

        if(left_hit && !right_hit) {
            // only left bbox hit
            hit_helper(ray, left_node, ret);
        } else if(!left_hit && right_hit) {
            // only right bbox hit
            hit_helper(ray, right_node, ret);
        } else {
            // both hit - front-to-back traversal
            if(left_hit_times.x < right_hit_times.x) {
                // left bbox hit before right bbox
                hit_helper(ray, left_node, ret);
                if(!ret->hit || left_hit_times.x < ret->distance) {
                    // if ray didn't hit anything on left, or right is still closer than closest hit
                    // on left
                    hit_helper(ray, right_node, ret);
                }
            } else {
                // right bbox hit before left bbox
                hit_helper(ray, right_node, ret);
                if(!ret->hit || right_hit_times.x < ret->distance) {
                    // if ray didn't hit anything on right, or left is still closer than closest hit
                    // on right
                    hit_helper(ray, left_node, ret);
                }
            }
        }
    }
}

template<typename Primitive>
BVH<Primitive>::BVH(std::vector<Primitive>&& prims, size_t max_leaf_size) {
    build(std::move(prims), max_leaf_size);
}

template<typename Primitive>
BVH<Primitive> BVH<Primitive>::copy() const {
    BVH<Primitive> ret;
    ret.nodes = nodes;
    ret.primitives = primitives;
    ret.root_idx = root_idx;
    return ret;
}

template<typename Primitive>
bool BVH<Primitive>::Node::is_leaf() const {
    return l == r;
}

template<typename Primitive>
size_t BVH<Primitive>::new_node(BBox box, size_t start, size_t size, size_t l, size_t r) {
    Node n;
    n.bbox = box;
    n.start = start;
    n.size = size;
    n.l = l;
    n.r = r;
    nodes.push_back(n);
    return nodes.size() - 1;
}

template<typename Primitive>
BBox BVH<Primitive>::bbox() const {
    return nodes[root_idx].bbox;
}

template<typename Primitive>
std::vector<Primitive> BVH<Primitive>::destructure() {
    nodes.clear();
    return std::move(primitives);
}

template<typename Primitive>
void BVH<Primitive>::clear() {
    nodes.clear();
    primitives.clear();
}

template<typename Primitive>
size_t BVH<Primitive>::visualize(GL::Lines& lines, GL::Lines& active, size_t level,
                                 const Mat4& trans) const {

    std::stack<std::pair<size_t, size_t>> tstack;
    tstack.push({root_idx, 0});
    size_t max_level = 0;

    if(nodes.empty()) return max_level;

    while(!tstack.empty()) {

        auto [idx, lvl] = tstack.top();
        max_level = std::max(max_level, lvl);
        const Node& node = nodes[idx];
        tstack.pop();

        Vec3 color = lvl == level ? Vec3(1.0f, 0.0f, 0.0f) : Vec3(1.0f);
        GL::Lines& add = lvl == level ? active : lines;

        BBox box = node.bbox;
        box.transform(trans);
        Vec3 min = box.min, max = box.max;

        auto edge = [&](Vec3 a, Vec3 b) { add.add(a, b, color); };

        edge(min, Vec3{max.x, min.y, min.z});
        edge(min, Vec3{min.x, max.y, min.z});
        edge(min, Vec3{min.x, min.y, max.z});
        edge(max, Vec3{min.x, max.y, max.z});
        edge(max, Vec3{max.x, min.y, max.z});
        edge(max, Vec3{max.x, max.y, min.z});
        edge(Vec3{min.x, max.y, min.z}, Vec3{max.x, max.y, min.z});
        edge(Vec3{min.x, max.y, min.z}, Vec3{min.x, max.y, max.z});
        edge(Vec3{min.x, min.y, max.z}, Vec3{max.x, min.y, max.z});
        edge(Vec3{min.x, min.y, max.z}, Vec3{min.x, max.y, max.z});
        edge(Vec3{max.x, min.y, min.z}, Vec3{max.x, max.y, min.z});
        edge(Vec3{max.x, min.y, min.z}, Vec3{max.x, min.y, max.z});

        if(node.l && node.r) {
            tstack.push({node.l, lvl + 1});
            tstack.push({node.r, lvl + 1});
        } else {
            for(size_t i = node.start; i < node.start + node.size; i++) {
                size_t c = primitives[i].visualize(lines, active, level - lvl, trans);
                max_level = std::max(c, max_level);
            }
        }
    }
    return max_level;
}

} // namespace PT
