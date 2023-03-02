
#include "../rays/bvh.h"
#include "debug.h"
#include <iostream>
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

    // // compute bounding box for all primitives
    // BBox bb;
    // for(size_t i = 0; i < primitives.size(); ++i) {
    //     bb.enclose(primitives[i].bbox());
    // }

    // // set up root node (root BVH). Notice that it contains all primitives.
    // size_t root_node_addr = new_node();
    // Node& node = nodes[root_node_addr];
    // node.bbox = bb;
    // node.start = 0;
    // node.size = primitives.size();

    // // Create bounding boxes for children
    // BBox split_leftBox;
    // BBox split_rightBox;

    // // compute bbox for left child
    // Primitive& p = primitives[0];
    // BBox pbb = p.bbox();
    // split_leftBox.enclose(pbb);

    // // compute bbox for right child
    // for(size_t i = 1; i < primitives.size(); ++i) {
    //     BBox pbb = primitives[i].bbox();
    //     split_rightBox.enclose(pbb);
    // }

    // // Note that by construction in this simple example, the primitives are
    // // contiguous as required. But in the students real code, students are
    // // responsible for reorganizing the primitives in the primitives array so that
    // // after a SAH split is computed, the chidren refer to contiguous ranges of primitives.

    // size_t startl = 0;                          // starting prim index of left child
    // size_t rangel = 1;                          // number of prims in left child
    // size_t startr = startl + rangel;            // starting prim index of right child
    // size_t ranger = primitives.size() - rangel; // number of prims in right child

    // // create child nodes
    // size_t node_addr_l = new_node();
    // size_t node_addr_r = new_node();
    // nodes[root_node_addr].l = node_addr_l;
    // nodes[root_node_addr].r = node_addr_r;

    // nodes[node_addr_l].bbox = split_leftBox;
    // nodes[node_addr_l].start = startl;
    // nodes[node_addr_l].size = rangel;

    // nodes[node_addr_r].bbox = split_rightBox;
    // nodes[node_addr_r].start = startr;
    // nodes[node_addr_r].size = ranger;

    // compute bounding box for all primitives
    BBox bbox;
    for(size_t i = 0; i < primitives.size(); ++i) {
        bbox.enclose(primitives[i].bbox());
    }
    std::cout << "primitives.size() " << primitives.size() << "\n";

    size_t node_idx = new_node(bbox, 0, primitives.size(), 0, 0);
    build_helper(node_idx, max_leaf_size);
}

template<typename Primitive> void BVH<Primitive>::build_helper(int node_idx, size_t max_leaf_size) {

    Node& node = nodes[node_idx];
    std::cout << "\n";

    // check if node is a leaf
    if(node.size <= max_leaf_size) {
        std::cout << "leaf"
                  << "\n";
        return;
    }

    std::cout << "node_idx " << node_idx << "\n";
    std::cout << "max_leaf_size " << max_leaf_size << "\n";

    size_t num_buckets = 16;
    BBox node_bbox = node.bbox;
    float node_surface_area = node_bbox.surface_area();
    Vec3 node_space = node_bbox.max - node_bbox.min;

    // for each axis: x, y, z:
    // initialize bucket counts to 0, per-bucket bboxes to empty
    std::vector<Bucket> buckets_x(num_buckets);
    std::vector<Bucket> buckets_y(num_buckets);
    std::vector<Bucket> buckets_z(num_buckets);

    // for each primitiive p in node:
    for(size_t i = node.start; i < node.start + node.size; i++) {
        // compute bucket using p.centroid
        BBox p_bbox = primitives[i].bbox();
        Vec3 p_space = (p_bbox.center() - node_bbox.min);

        Vec3 bucket_index = p_space / node_space * num_buckets;
        size_t bucket_index_x = bucket_index.x;
        size_t bucket_index_y = bucket_index.y;
        size_t bucket_index_z = bucket_index.z;

        // add primitive to bbox union
        buckets_x[bucket_index_x].bbox.enclose(p_bbox);
        buckets_y[bucket_index_y].bbox.enclose(p_bbox);
        buckets_z[bucket_index_z].bbox.enclose(p_bbox);

        // increment bucket count of primitives
        buckets_x[bucket_index_x].primitives_count += 1;
        buckets_y[bucket_index_y].primitives_count += 1;
        buckets_z[bucket_index_z].primitives_count += 1;
    }

    // for(size_t i = 0; i < num_buckets; i++) {
    //     std::cout << "ITERATION " << i << "\n";
    //     std::cout << buckets_x[i].primitives_count << "\n";
    //     std::cout << buckets_x[i].bbox.surface_area() << "\n";
    //     std::cout << buckets_y[i].primitives_count << "\n";
    //     std::cout << buckets_y[i].bbox.surface_area() << "\n";
    //     std::cout << buckets_z[i].primitives_count << "\n";
    //     std::cout << buckets_z[i].bbox.surface_area() << "\n";

    //     std::cout << "\n";
    // }

    // for each of the possible b-1 partition planes, evaluate SAH and find lowest cost partition
    float left_size = 0;
    float lowest_cost = FLT_MAX;
    for(size_t i = 1; i < num_buckets; i++) {
        Bucket left_partition_buckets_x;
        Bucket left_partition_buckets_y;
        Bucket left_partition_buckets_z;

        Bucket right_partition_buckets_x;
        Bucket right_partition_buckets_y;
        Bucket right_partition_buckets_z;

        // make the two parititions to evaluate cost on
        for(size_t j = 0; j < num_buckets; j++) {
            Bucket bucket_x = buckets_x[j];
            Bucket bucket_y = buckets_y[j];
            Bucket bucket_z = buckets_z[j];

            if(j < i) {
                left_partition_buckets_x.bbox.enclose(bucket_x.bbox);
                left_partition_buckets_y.bbox.enclose(bucket_y.bbox);
                left_partition_buckets_z.bbox.enclose(bucket_z.bbox);

                left_partition_buckets_x.primitives_count += bucket_x.primitives_count;
                left_partition_buckets_y.primitives_count += bucket_y.primitives_count;
                left_partition_buckets_z.primitives_count += bucket_z.primitives_count;
            } else {
                right_partition_buckets_x.bbox.enclose(bucket_x.bbox);
                right_partition_buckets_y.bbox.enclose(bucket_y.bbox);
                right_partition_buckets_z.bbox.enclose(bucket_z.bbox);

                right_partition_buckets_x.primitives_count += bucket_x.primitives_count;
                right_partition_buckets_y.primitives_count += bucket_y.primitives_count;
                right_partition_buckets_z.primitives_count += bucket_z.primitives_count;
            }
        }

        // evaluate SAH = S_A / S_N * num_prim_A + S_B / S_N * num_prim_B
        float cost_x = left_partition_buckets_x.bbox.surface_area() / node_surface_area *
                           left_partition_buckets_x.primitives_count +
                       right_partition_buckets_x.bbox.surface_area() / node_surface_area *
                           right_partition_buckets_x.primitives_count;

        float cost_y = left_partition_buckets_y.bbox.surface_area() / node_surface_area *
                           left_partition_buckets_y.primitives_count +
                       right_partition_buckets_y.bbox.surface_area() / node_surface_area *
                           right_partition_buckets_y.primitives_count;

        float cost_z = left_partition_buckets_z.bbox.surface_area() / node_surface_area *
                           left_partition_buckets_z.primitives_count +
                       right_partition_buckets_z.bbox.surface_area() / node_surface_area *
                           right_partition_buckets_z.primitives_count;

        // std::cout << "cost_x " << cost_x << "\n";
        // std::cout << "cost_y " << cost_y << "\n";
        // std::cout << "cost_z " << cost_z << "\n";

        float potential_lowest_cost = std::min(std::min(cost_x, cost_y), cost_z);
        // std::cout << "potential_lowest_cost " << potential_lowest_cost << "\n";

        if(potential_lowest_cost < lowest_cost) {
            lowest_cost = potential_lowest_cost;
            left_size = i;
        }
    }

    // edge case - lowest cost partition is no partition, just split down the middle
    if(left_size == 0) {
        left_size = node.size / 2;
    }

    std::cout << "left_size " << left_size << "\n";

    // create bounding boxes for children
    BBox split_left_bbox;
    for(size_t i = 0; i < left_size; i++) {
        BBox primitive_bbox = primitives[node.start + i].bbox();
        split_left_bbox.enclose(primitive_bbox);
    }
    size_t left_idx = new_node(split_left_bbox, node.start, left_size, 0, 0);

    BBox split_right_bbox;
    for(size_t i = left_size; i < node.size; i++) {
        BBox primitive_bbox = primitives[node.start + i].bbox();
        split_right_bbox.enclose(primitive_bbox);
    }
    size_t right_size = node.size - left_size;
    size_t right_idx = new_node(split_right_bbox, node.start + left_size, right_size, 0, 0);

    // update parent node
    node.l = left_idx;
    node.r = right_idx;

    std::cout << "node left " << node.l << "\n";
    std::cout << "node right " << node.r << "\n";

    // use lowest cost partition found and recurse on that split
    build_helper(left_idx, max_leaf_size);
    build_helper(right_idx, max_leaf_size);
}

template<typename Primitive> Trace BVH<Primitive>::hit(const Ray& ray) const {

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

    if(node.is_leaf()) {
        for(size_t i = node.start; i < node.start + node.size; i++) {
            Trace hit = primitives[i].hit(ray);
            *ret = Trace::min(*ret, hit);
        }
        return;
    }

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
            if(!ret->hit || right_hit_times.x < ret->distance) {
                // if ray didn't hit anything on left, or right is still closer than closest hit
                // on left
                hit_helper(ray, right_node, ret);
            }
        } else {
            // right bbox hit before left bbox
            hit_helper(ray, right_node, ret);
            if(!ret->hit || left_hit_times.x < ret->distance) {
                // if ray didn't hit anything on right, or left is still closer than closest hit
                // on right
                hit_helper(ray, left_node, ret);
            }
        }
    }
}

template<typename Primitive>
BVH<Primitive>::BVH(std::vector<Primitive>&& prims, size_t max_leaf_size) {
    build(std::move(prims), max_leaf_size);
}

template<typename Primitive> BVH<Primitive> BVH<Primitive>::copy() const {
    BVH<Primitive> ret;
    ret.nodes = nodes;
    ret.primitives = primitives;
    ret.root_idx = root_idx;
    return ret;
}

template<typename Primitive> bool BVH<Primitive>::Node::is_leaf() const {
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

template<typename Primitive> BBox BVH<Primitive>::bbox() const {
    return nodes[root_idx].bbox;
}

template<typename Primitive> std::vector<Primitive> BVH<Primitive>::destructure() {
    nodes.clear();
    return std::move(primitives);
}

template<typename Primitive> void BVH<Primitive>::clear() {
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
