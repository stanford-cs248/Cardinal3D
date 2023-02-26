
#include "../rays/tri_mesh.h"
#include "debug.h"

namespace PT {

BBox Triangle::bbox() const {

    // TODO (PathTracer): Task 2
    // compute the bounding box of the triangle

    // Beware of flat/zero-volume boxes! You may need to
    // account for that here, or later on in BBox::intersect

    BBox box;

    box.enclose(vertex_list[v0].position);
    box.enclose(vertex_list[v1].position);
    box.enclose(vertex_list[v2].position);

    return box;
}

Trace Triangle::hit(const Ray& ray) const {

    // Vertices of triangle - has postion and surface normal
    // See rays/tri_mesh.h for a description of this struct

    Tri_Mesh_Vert v_0 = vertex_list[v0];
    Tri_Mesh_Vert v_1 = vertex_list[v1];
    Tri_Mesh_Vert v_2 = vertex_list[v2];

    // TODO (PathTracer): Task 2
    // Intersect this ray with a triangle defined by the above three points.
    // Intersection should yield a ray t-value, and a hit point (u,v) on the surface of the triangle
    // You'll need to fill in a "Trace" struct describing information about the hit (or lack of hit)

    // get relevant variables for ray equation
    Vec3 p_0 = v_0.position;
    Vec3 p_1 = v_1.position;
    Vec3 p_2 = v_2.position;
    Vec3 o = ray.point;
    Vec3 d = ray.dir;

    // get uv-plane variables
    Vec3 e_1 = p_1 - p_0;
    Vec3 e_2 = p_2 - p_0;
    Vec3 s = o - p_0;

    // cross product sub-expressions
    Vec3 e_1_cross_d = cross(e_1, d);
    Vec3 s_cross_e_2 = cross(s, e_2);

    // dot product sub-expressions
    float e_1_cross_d_dot_e_2 = dot(e_1_cross_d, e_2);
    float s_cross_e_2_dot_d = dot(s_cross_e_2, d);
    float e_1_cross_d_dot_s = dot(e_1_cross_d, s);
    float s_cross_e_2_dot_e_1 = dot(s_cross_e_2, e_1);

    // if denominator is 0, no intersection found
    if(e_1_cross_d_dot_e_2 == 0) {
        return Trace();
    }

    Vec3 vec = Vec3(-1.0 * s_cross_e_2_dot_d, e_1_cross_d_dot_s, -1.0 * s_cross_e_2_dot_e_1);
    Vec3 u_v_t = 1.0 / e_1_cross_d_dot_e_2 * vec;

    float t = u_v_t.z;

    // barycentric coordinates sum to 1
    float u = u_v_t.x;
    float v = u_v_t.y;
    float w = 1 - (u + v);

    // barycentric coordinates must be exclusively 0 and 1 -> (0, 1)
    if(!(u > 0 && u < 1) || !(v > 0 && v < 1) || !(v > 0 && v < 1)) {
        return Trace();
    }

    // intersection point must be within bounds of the ray
    if(t < ray.dist_bounds[0] || t > ray.dist_bounds[1]) {
        return Trace();
    }

    // normal obtained via interpolation of the per-vertex normals according to the barycentric
    // coordinates of the hit point
    Vec3 n_0 = v_0.normal;
    Vec3 n_1 = v_1.normal;
    Vec3 n_2 = v_2.normal;
    Vec3 normal = n_0 * w + n_1 * u + p_2 * v;

    Trace ret;
    ret.origin = ray.point;
    ret.hit = true;           // was there an intersection?
    ret.distance = t;         // at what distance did the intersection occur?
    ret.position = ray.at(t); // where was the intersection?
    ret.normal = normal;      // what was the surface normal at the intersection?
                              // (this should be interpolated between the three vertex normals)
    return ret;
}

Triangle::Triangle(Tri_Mesh_Vert* verts, unsigned int v0, unsigned int v1, unsigned int v2)
    : vertex_list(verts), v0(v0), v1(v1), v2(v2) {
}

void Tri_Mesh::build(const GL::Mesh& mesh) {

    verts.clear();
    triangles.clear();

    for(const auto& v : mesh.verts()) {
        verts.push_back({v.pos, v.norm});
    }

    const auto& idxs = mesh.indices();

    std::vector<Triangle> tris;
    for(size_t i = 0; i < idxs.size(); i += 3) {
        tris.push_back(Triangle(verts.data(), idxs[i], idxs[i + 1], idxs[i + 2]));
    }

    triangles.build(std::move(tris), 4);
}

Tri_Mesh::Tri_Mesh(const GL::Mesh& mesh) {
    build(mesh);
}

Tri_Mesh Tri_Mesh::copy() const {
    Tri_Mesh ret;
    ret.verts = verts;
    ret.triangles = triangles.copy();
    return ret;
}

BBox Tri_Mesh::bbox() const {
    return triangles.bbox();
}

Trace Tri_Mesh::hit(const Ray& ray) const {
    Trace t = triangles.hit(ray);
    return t;
}

size_t Tri_Mesh::visualize(GL::Lines& lines, GL::Lines& active, size_t level,
                           const Mat4& trans) const {
    return triangles.visualize(lines, active, level, trans);
}

} // namespace PT
