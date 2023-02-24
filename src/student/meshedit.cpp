#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <unordered_map>

#include "../geometry/halfedge.h"
#include "debug.h"
#include <unordered_set>

/* Note on local operation return types:
    The local operations all return a std::optional<T> type. This is used so that your
    implementation can signify that it does not want to perform the operation for
    whatever reason (e.g. you don't want to allow the user to erase the last vertex).
    An optional can have two values: std::nullopt, or a value of the type it is
    parameterized on. In this way, it's similar to a pointer, but has two advantages:
    the value it holds need not be allocated elsewhere, and it provides an API that
    forces the user to check if it is null before using the value.
    In your implementaiton, if you have successfully performed the operation, you can
    simply return the required reference:
            ... collapse the edge ...
            return collapsed_vertex_ref;
    And if you wish to deny the operation, you can return the null optional:
            return std::nullopt;
    Note that the stubs below all reject their duties by returning the null optional.
*/

/*
    This method should replace the given vertex and all its neighboring
    edges and faces with a single face, returning the new face.
 */
std::optional<Halfedge_Mesh::FaceRef> Halfedge_Mesh::erase_vertex(Halfedge_Mesh::VertexRef v) {

    std::vector<std::vector<HalfedgeRef>> all_halfedges;
    std::vector<EdgeRef> del_edges;
    std::vector<HalfedgeRef> del_halfedges;
    std::vector<FaceRef> faces;

    // iterate through edges of vertex
    HalfedgeRef h = v->halfedge();

    do {
        std::vector<HalfedgeRef> polygon;

        // iterate through halfedges within each face attached to vertex
        HalfedgeRef h_inner = h;
        do {
            polygon.push_back(h_inner);
            h_inner = h_inner->next();
        } while(h_inner != h);

        del_edges.push_back(h->edge());
        faces.push_back(h->face());
        all_halfedges.push_back(polygon);

        // go to next edge
        h = h->twin()->next();
    } while(h != v->halfedge());

    int num_faces = (int)all_halfedges.size();
    for(int i = 0; i < num_faces; i++) {
        std::vector<HalfedgeRef> curr, prev, next;
        curr = all_halfedges[i];
        prev = all_halfedges[(i - 1 + num_faces) % num_faces];

        HalfedgeRef next_link = prev[1];
        faces[0]->halfedge() = next_link;
        // set first degree vertex to curr[1] to unlink it from the edge to delete
        curr[0]->twin()->vertex()->halfedge() = curr[1];

        // add first and last halfedge to delete list
        del_halfedges.push_back(curr[0]);
        del_halfedges.push_back(curr[curr.size() - 1]);

        // add the edge to delete list
        del_edges.push_back(curr[0]->edge());

        // iterate through middle halfedges and set their faces to faces[0]
        for(int i = 1; i < (int)curr.size() - 1; i++) {
            HalfedgeRef next = (i == (int)curr.size() - 2) ? next_link : curr[i]->next();
            curr[i]->set_neighbors(next, curr[i]->twin(), curr[i]->vertex(), curr[i]->edge(),
                                   faces[0]);
        }
    }

    // delete all faces except first one, edges to delete, and halfedges to delete
    for(int i = 0; i < (int)faces.size(); i++) {
        if(i != 0) {
            erase(faces[i]);
        }
        erase(del_edges[i]);
        erase(del_halfedges[i * 2]);
        erase(del_halfedges[i * 2 + 1]);
    }
    erase(v);

    return faces[0];
}

/*
    This method should erase the given edge and return an iterator to the
    merged face.
 */
std::optional<Halfedge_Mesh::FaceRef> Halfedge_Mesh::erase_edge(Halfedge_Mesh::EdgeRef e) {

    std::vector<HalfedgeRef> polygon1, polygon2;
    HalfedgeRef h = e->halfedge();
    HalfedgeRef twin = h->twin();

    FaceRef f1 = h->face();
    FaceRef f2 = twin->face();

    VertexRef v1 = h->vertex();
    VertexRef v2 = twin->vertex();

    do {
        polygon1.push_back(h);
        h = h->next();
    } while(h != e->halfedge());

    do {
        polygon2.push_back(twin);

        // set neighbors in the face to delete to the other face
        twin->set_neighbors(twin->next(), twin->twin(), twin->vertex(), twin->edge(), f1);
        twin = twin->next();
    } while(twin != e->halfedge()->twin());

    HalfedgeRef h1 = polygon1[0];
    HalfedgeRef h2 = polygon2[0];

    HalfedgeRef h1_next = polygon1[1];
    HalfedgeRef h2_next = polygon2[1];

    HalfedgeRef h1_prev = polygon1.back();
    HalfedgeRef h2_prev = polygon2.back();

    // reassignment of halfedges
    f1->halfedge() = polygon1[1];
    v1->halfedge() = polygon2[1];
    v2->halfedge() = polygon1[1];

    // set new neighbors surrounding edge to delete
    h1_prev->set_neighbors(h2_next, h1_prev->twin(), h1_prev->vertex(), h1_prev->edge(), f1);
    h2_prev->set_neighbors(h1_next, h2_prev->twin(), h2_prev->vertex(), h2_prev->edge(), f1);

    erase(f2);
    erase(e);
    erase(h1);
    erase(h2);
    return f1;
}

/*
    This method should collapse the given edge and return an iterator to
    the new vertex created by the collapse.
*/
std::optional<Halfedge_Mesh::VertexRef> Halfedge_Mesh::collapse_edge(Halfedge_Mesh::EdgeRef e) {

    if(e->on_boundary()) {
        std::cout << "edge is on boundary, don't do this operation";
        return std::nullopt;
    }

    // 0. get all relevant metadata
    HalfedgeRef he_0 = e->halfedge();
    HalfedgeRef he_0_iter = he_0->next();
    FaceRef f_0 = he_0->face();
    VertexRef v_0 = he_0->vertex();

    HalfedgeRef he_1 = he_0->twin();
    HalfedgeRef he_1_iter = he_1->next();
    FaceRef f_1 = he_1->face();
    VertexRef v_1 = he_1->vertex();

    // 1. find n-polygon face size in # edges by iterating through half edges on a single face
    // -> get half edges and face sizes (num_edges_f_i), save he_i_prev
    int num_edges_f_0 = 1;
    HalfedgeRef he_0_prev;
    while(he_0_iter != he_0) {
        num_edges_f_0 += 1;
        he_0_prev = he_0_iter;

        he_0_iter = he_0_iter->next();
    }

    int num_edges_f_1 = 1;
    HalfedgeRef he_1_prev;
    while(he_1_iter != he_1) {
        num_edges_f_1 += 1;
        he_1_prev = he_1_iter;

        he_1_iter = he_1_iter->next();
    }

    // 2. loop through the vertex's half edges (v_1) and reassign them to the kept vertex v_0, then
    // erase v_1
    he_1_iter = he_1_iter->twin()->next();
    while(he_1_iter != he_1) {
        he_1_iter->set_neighbors(he_1_iter->next(), he_1_iter->twin(), v_0, he_1_iter->edge(),
                                 he_1_iter->face());

        he_1_iter = he_1_iter->twin()->next();
    }
    erase(v_1);

    // 3a. Assign pointers for halfedges, and vertices if needed. delete edges/faces if needed.
    if(num_edges_f_0 > 3) {
        // if not a triangle (edges > 3), set pointers to skip over half edge
        he_0_prev->next() = he_0->next();

        // update the face's halfedge since that halfedge will be deleted
        he_0->face()->halfedge() = he_0->next();
    } else if(num_edges_f_0 == 3) {
        // if a triangle (edges = 3), then check he_0->next()->vertex() is v_1 (vertex to remove).

        HalfedgeRef he_to_update;
        HalfedgeRef he_twin_on_other_polygon;
        HalfedgeRef neighbor_polygon_he_iter;
        HalfedgeRef other_he_to_update;
        VertexRef v_to_update;
        EdgeRef e_to_delete;

        // delete edge and it's half edges to avoid stacked edges
        e_to_delete = he_0->next()->edge();
        erase(e_to_delete);
        erase(e_to_delete->halfedge());
        erase(e_to_delete->halfedge()->twin());

        // update the face's halfedge since that halfedge will be deleted
        he_0->face()->halfedge() = he_0->next()->next();

        // update the neighboring face's halfedge since that halfedge (via twin) will be deleted
        he_0->next()->twin()->face()->halfedge() = he_0->next()->twin()->next();

        // need to update two half edges and one vertex since removing another edge
        he_to_update = he_0->next()->next();
        he_twin_on_other_polygon = he_0->next()->twin();
        neighbor_polygon_he_iter = he_twin_on_other_polygon->next();
        // need to get to the least HE on neighboring polygon who's next is the twin of the HE we
        // delete on the triangle (HE->next)
        while(neighbor_polygon_he_iter != he_twin_on_other_polygon) {
            other_he_to_update = neighbor_polygon_he_iter;
            neighbor_polygon_he_iter = neighbor_polygon_he_iter->next();
        }
        v_to_update = he_0->next()->twin()->vertex();

        // update next and face for halfedge on face to be removed
        he_to_update->set_neighbors(he_0->next()->twin()->next(), he_to_update->twin(),
                                    he_to_update->vertex(), he_to_update->edge(),
                                    he_0->next()->twin()->next()->face());

        // update next for halfedge on neighboring face
        other_he_to_update->set_neighbors(he_0->next()->next(), other_he_to_update->twin(),
                                          other_he_to_update->vertex(), other_he_to_update->edge(),
                                          other_he_to_update->face());

        // update halfedge for vertex whose edge will be removed
        v_to_update->halfedge() = he_0->next()->next();

        // delete the face
        erase(f_0);
    } else {
        // this isn't a polygon and so shouldn't be in the mesh, error.
        std::cout << "face 0 needs to have at least 3 edges to be a polygon";
        return std::nullopt;
    }

    // 3b. same for the other face f_1, might be different num_edges from f_0
    if(num_edges_f_1 > 3) {
        // if not a triangle (edges > 3), set pointers to skip over half edge
        he_1_prev->next() = he_1->next();

        // update the face's halfedge since that halfedge will be deleted
        he_1->face()->halfedge() = he_1->next();
    } else if(num_edges_f_1 == 3) {
        // if a triangle (edges = 3), then check he_1->next()->vertex() is v_1 (vertex to remove).
        HalfedgeRef he_to_update;
        HalfedgeRef he_twin_on_other_polygon;
        HalfedgeRef neighbor_polygon_he_iter;
        HalfedgeRef other_he_to_update;
        VertexRef v_to_update;
        EdgeRef e_to_delete;

        // delete edge and it's half edges to avoid stacked edges
        e_to_delete = he_1->next()->next()->edge();
        erase(e_to_delete);
        erase(e_to_delete->halfedge());
        erase(e_to_delete->halfedge()->twin());

        // update the face's halfedge since that halfedge (and the next->next) will be deleted
        he_1->face()->halfedge() = he_1->next();

        // update the neighboring face's halfedge since that halfedge (via twin) will be deleted
        he_1->next()->next()->twin()->face()->halfedge() = he_1->next()->next()->twin()->next();

        // need to update two half edges and one vertex since removing another edge
        he_to_update = he_1->next();
        he_twin_on_other_polygon = he_1->next()->next()->twin();
        neighbor_polygon_he_iter = he_twin_on_other_polygon->next();
        // need to get to the least HE on neighboring polygon who's next is the twin of the HE we
        // delete on the triangle (HE->next->next)
        while(neighbor_polygon_he_iter != he_twin_on_other_polygon) {
            other_he_to_update = neighbor_polygon_he_iter;
            neighbor_polygon_he_iter = neighbor_polygon_he_iter->next();
        }
        v_to_update = he_1->next()->next()->vertex();

        // make this work for neigbhoring non-triangles (iterate)

        // update next and face for halfedge on face to be removed
        he_to_update->set_neighbors(he_1->next()->next()->twin()->next(), he_to_update->twin(),
                                    he_to_update->vertex(), he_to_update->edge(),
                                    he_1->next()->next()->twin()->next()->face());

        // update next for halfedge on neighboring face
        other_he_to_update->set_neighbors(he_1->next(), other_he_to_update->twin(),
                                          other_he_to_update->vertex(), other_he_to_update->edge(),
                                          other_he_to_update->face());

        // update halfedge for vertex whose edge will be removed
        v_to_update->halfedge() = he_1->next()->twin();

        // delete the face
        erase(f_1);
    } else {
        // this isn't a polygon and so shouldn't be in the mesh, error.
        std::cout << "face 1 needs to have at least 3 edges to be a polygon";
        return std::nullopt;
    }

    // 4. reposition the v_0 to be at the midpoint of the edge. Also make sure to move update the
    // kept vertex's halfedge property in case it's the one being deleted.
    v_0->halfedge() = he_0->twin()->next();
    v_0->pos = e->center();

    // 5. Delete edge and its half edges to complete the collapse
    erase(e);
    erase(he_0);
    erase(he_1);

    return v_0;
}

/*
    This method should collapse the given face and return an iterator to
    the new vertex created by the collapse.
*/
std::optional<Halfedge_Mesh::VertexRef> Halfedge_Mesh::collapse_face(Halfedge_Mesh::FaceRef f) {
    std::vector<std::vector<HalfedgeRef>> all_halfedges;
    std::vector<EdgeRef> del_edges;
    std::vector<HalfedgeRef> del_halfedges;
    std::vector<VertexRef> del_vertices;
    std::vector<FaceRef> faces;

    Vec3 c = f->center();
    VertexRef v_c = new_vertex();
    v_c->pos = c;

    // iterate through edges of face
    HalfedgeRef h = f->halfedge();

    do {
        std::vector<HalfedgeRef> polygon;

        // iterate through halfedges within each face attached to vertex
        HalfedgeRef h_inner = h->twin();
        del_vertices.push_back(h_inner->vertex());
        do {
            polygon.push_back(h_inner);
            h_inner = h_inner->next();
        } while(h_inner != h->twin());

        del_edges.push_back(h->edge());
        faces.push_back(h->face());
        del_halfedges.push_back(h);
        del_halfedges.push_back(h->twin());
        all_halfedges.push_back(polygon);

        // go to next edge
        h = h->next();
    } while(h != f->halfedge());

    int size = del_edges.size();
    for (int i = 0; i < size; i++) {
        std::vector<HalfedgeRef> curr = all_halfedges[i];

        // ensure face is assigned to live halfedge
        faces[i]->halfedge() = curr[1];
        v_c->halfedge() = curr[1];
        
        HalfedgeRef first = curr[1];
        HalfedgeRef last = curr.back();
        // set neighbors
        first->set_neighbors(first->next(), first->twin(), v_c, first->edge(), first->face());
        last->set_neighbors(curr[1], last->twin(), last->vertex(), last->edge(), last->face());
    }

    // delete all faces except first one, edges to delete, and halfedges to delete
    for(int i = 0; i < size; i++) {
        erase(f);
        erase(del_edges[i]);
        erase(del_vertices[i]);
        erase(del_halfedges[i * 2]);
        erase(del_halfedges[i * 2 + 1]);
    }
    
    return v_c;
}

/*
    This method should flip the given edge and return an iterator to the
    flipped edge.
*/
std::optional<Halfedge_Mesh::EdgeRef> Halfedge_Mesh::flip_edge(Halfedge_Mesh::EdgeRef e) {
    HalfedgeRef h1 = e->halfedge();
    HalfedgeRef h2 = h1->twin();

    FaceRef f1 = h1->face();
    FaceRef f2 = h2->face();

    HalfedgeRef h1_next = h1->next();
    HalfedgeRef h2_next = h2->next();

    HalfedgeRef h1_next2 = h1_next->next();
    HalfedgeRef h2_next2 = h2_next->next();

    VertexRef v1 = h1_next2->vertex();
    VertexRef v2 = h2_next2->vertex();

    h1->set_neighbors(h1_next2, h2, v2, e, f1);
    h2->set_neighbors(h2_next2, h1, v1, e, f2);
    h1_next->set_neighbors(h2, h1_next->twin(), h1_next->vertex(), h1_next->edge(), f2);
    h2_next->set_neighbors(h1, h2_next->twin(), h2_next->vertex(), h2_next->edge(), f1);

    HalfedgeRef lasth1 = h1;
    HalfedgeRef lasth2 = h2;
    do {
        lasth1 = lasth1->next();
    } while(lasth1->next() != h1);

    do {
        lasth2 = lasth2->next();
    } while(lasth2->next() != h2);

    lasth1->set_neighbors(h2_next, lasth1->twin(), lasth1->vertex(), lasth1->edge(), f1);
    lasth2->set_neighbors(h1_next, lasth2->twin(), lasth2->vertex(), lasth2->edge(), f2);

    f1->halfedge() = h1;
    f2->halfedge() = h2;

    return e;
}

/*
    This method should split the given edge and return an iterator to the
    newly inserted vertex. The halfedge of this vertex should point along
    the edge that was split, rather than the new edges.
*/
std::optional<Halfedge_Mesh::VertexRef> Halfedge_Mesh::split_edge(Halfedge_Mesh::EdgeRef e) {
    // get center of edge
    Vec3 new_v_pos = e->center();
    // create new Vertex and move pos to center
    VertexRef new_v = new_vertex();
    new_v->pos = new_v_pos;
    new_v->is_new = true;

    HalfedgeRef he1 = e->halfedge();
    HalfedgeRef he2 = e->halfedge()->twin();

    // splt main edge e, new branch is associated with he2, old branch is associated with he1
    EdgeRef trunk_e = new_edge();
    trunk_e->halfedge() = he2;
    he2->edge() = trunk_e;
    e->halfedge() = he1;

    for(HalfedgeRef he : {he1, he2}) {
        // store he_face = e.face
        FaceRef he_face = he->face();

        // store he_twin = e.halfedge.twin
        HalfedgeRef he_twin = he->twin();

        // store he_next = e.halfedge.next
        HalfedgeRef he_next = he->next();

        // store he_next2 = e.halfedge.next.next
        HalfedgeRef he_next2 = he->next()->next();
        he_face->halfedge() = he_next2;

        // create new_he1
        // create new_he2
        // create new_he3
        HalfedgeRef new_he1 = new_halfedge();
        new_v->halfedge() = new_he1;
        HalfedgeRef new_he2 = new_halfedge();
        HalfedgeRef new_he3 = new_halfedge();

        // create new_face
        FaceRef new_f = new_face();
        new_f->halfedge() = he_next;

        // create branch_e
        EdgeRef branch_e = new_edge();
        branch_e->halfedge() = new_he2;
        branch_e->is_new = true;

        new_he1->set_neighbors(he_next, he_twin, new_v, he_twin->edge(), new_f);

        he_next->set_neighbors(new_he2, he_next->twin(), he_next->vertex(), he_next->edge(), new_f);

        new_he2->set_neighbors(new_he1, new_he3, he_next2->vertex(), branch_e, new_f);

        new_he3->set_neighbors(he_next2, new_he2, new_v, branch_e, he_face);

        // ignore twin for now, will set outside for loop because loop uses twin
        he->set_neighbors(new_he3, he->twin(), he->vertex(), he->edge(), he_face);
    }

    // set twin for the original half edges
    he1->set_neighbors(he1->next(), he2->next()->twin()->next(), he1->vertex(), he1->edge(),
                       he1->face());

    he2->set_neighbors(he2->next(), he1->next()->twin()->next(), he2->vertex(), he2->edge(),
                       he2->face());

    return new_v;
}

/* Note on the beveling process:
    Each of the bevel_vertex, bevel_edge, and bevel_face functions do not represent
    a full bevel operation. Instead, they should update the _connectivity_ of
    the mesh, _not_ the positions of newly created vertices. In fact, you should set
    the positions of new vertices to be exactly the same as wherever they "started from."
    When you click on a mesh element while in bevel mode, one of those three functions
    is called. But, because you may then adjust the distance/offset of the newly
    beveled face, we need another method of updating the positions of the new vertices.
    This is where bevel_vertex_positions, bevel_edge_positions, and
    bevel_face_positions come in: these functions are called repeatedly as you
    move your mouse, the position of which determins the normal and tangent offset
    parameters. These functions are also passed an array of the original vertex
    positions: for  bevel_vertex, it has one element, the original vertex position,
    for bevel_edge,  two for the two vertices, and for bevel_face, it has the original
    position of each vertex in halfedge order. You should use these positions, as well
    as the normal and tangent offset fields to assign positions to the new vertices.
    Finally, note that the normal and tangent offsets are not relative values - you
    should compute a particular new position from them, not a delta to apply.
*/

/*
    This method should replace the vertex v with a face, corresponding to
    a bevel operation. It should return the new face.  NOTE: This method is
    responsible for updating the *connectivity* of the mesh only---it does not
    need to update the vertex positions.  These positions will be updated in
    Halfedge_Mesh::bevel_vertex_positions (which you also have to
    implement!)
*/
std::optional<Halfedge_Mesh::FaceRef> Halfedge_Mesh::bevel_vertex(Halfedge_Mesh::VertexRef v) {

    // Reminder: You should set the positions of new vertices (v->pos) to be exactly
    // the same as wherever they "started from."

    (void)v;
    return std::nullopt;
}

/*
    This method should replace the edge e with a face, corresponding to a
    bevel operation. It should return the new face. NOTE: This method is
    responsible for updating the *connectivity* of the mesh only---it does not
    need to update the vertex positions.  These positions will be updated in
    Halfedge_Mesh::bevel_edge_positions (which you also have to
    implement!)
*/
std::optional<Halfedge_Mesh::FaceRef> Halfedge_Mesh::bevel_edge(Halfedge_Mesh::EdgeRef e) {

    // Reminder: You should set the positions of new vertices (v->pos) to be exactly
    // the same as wherever they "started from."

    (void)e;
    return std::nullopt;
}

/*
    This method should replace the face f with an additional, inset face
    (and ring of faces around it), corresponding to a bevel operation. It
    should return the new face.  NOTE: This method is responsible for updating
    the *connectivity* of the mesh only---it does not need to update the vertex
    positions. These positions will be updated in
    Halfedge_Mesh::bevel_face_positions (which you also have to
    implement!)
*/
std::optional<Halfedge_Mesh::FaceRef> Halfedge_Mesh::bevel_face(Halfedge_Mesh::FaceRef f) {

    // Reminder: You should set the positions of new vertices (v->pos) to be exactly
    // the same as wherever they "started from."
    unsigned int degree = f->degree();

    // vectors for storing new elements in order
    std::vector<VertexRef> old_vertices = {};
    std::vector<VertexRef> new_vertices = {};
    std::vector<HalfedgeRef> inner_halfedges = {};
    std::vector<HalfedgeRef> outer_halfedges = {};
    std::vector<EdgeRef> inner_edges = {};
    std::vector<EdgeRef> outer_edges = {};
    std::vector<HalfedgeRef> branch_halfedges = {};
    std::vector<EdgeRef> branch_edges = {};
    std::vector<FaceRef> side_faces = {};

    // iterate through and create new face, 2 new edges, 4 new he's, a new vertex
    HalfedgeRef he = f->halfedge();
    do {
        old_vertices.push_back(he->next()->vertex());
        new_vertices.push_back(new_vertex());
        inner_halfedges.insert(inner_halfedges.end(), {new_halfedge(), new_halfedge()});
        outer_halfedges.insert(outer_halfedges.end(), {he, he->twin()});
        inner_edges.push_back(new_edge());
        branch_halfedges.insert(branch_halfedges.end(), {new_halfedge(), new_halfedge()});
        branch_edges.push_back(new_edge());
        side_faces.push_back(new_face());

        he = he->next();

    } while(he != f->halfedge());

    // old_vertices = {old_v, ...} --size n
    // new_vertices = {new_v, ...}
    // inner_halfedges = {inner_he1, inner_he2, ...} --size
    // outer_halfedges = {outer_he1, outer_he2, ...}
    // inner_edges = {inner_e, ...}
    // branch_halfedges = {branch_he1, branch_he2, ...} -- size 6, i = 1, vertex 0, (i-1-1) %
    // (degree * 2) branch_edges = {branch_e, ...} side_faces = {side_f, ...}
    for(unsigned int i = 1; i <= degree; i++) {
        VertexRef old_v = old_vertices[i - 1];
        VertexRef new_v = new_vertices[i - 1];
        new_v->pos = old_v->pos;

        HalfedgeRef inner_he1 = inner_halfedges[i * 2 - 2];
        HalfedgeRef inner_he2 = inner_halfedges[i * 2 - 1];
        HalfedgeRef outer_he1 = outer_halfedges[i * 2 - 2];

        EdgeRef inner_e = inner_edges[i - 1];

        HalfedgeRef branch_he1 = branch_halfedges[i * 2 - 2];
        HalfedgeRef branch_he2 = branch_halfedges[i * 2 - 1];

        EdgeRef branch_e = branch_edges[i - 1];

        FaceRef side_f = side_faces[i - 1];

        // setting properties for vertices, faces, edges
        old_v->halfedge() = branch_he1;
        new_v->halfedge() = inner_he1;
        branch_e->halfedge() = branch_he1;
        inner_e->halfedge() = inner_he1;
        side_f->halfedge() = branch_he1;
        f->halfedge() = inner_he2;

        // calculating ideal indices
        int last_of_n = ((i - 1) - 1 + degree) % degree;
        int last_of_2n = (((i - 1) * 2 - 1) + (degree * 2)) % (degree * 2);
        int next_of_n = (i + degree) % degree;
        int next_of_2n = ((i * 2) + (degree * 2)) % (degree * 2);

        // setting halfedge neighbors (next, twin, v, e, f) -- need to complete and need to mod
        inner_he1->set_neighbors(branch_halfedges[last_of_2n], inner_he2, new_v, inner_e, side_f);
        inner_he2->set_neighbors(inner_halfedges[next_of_2n + 1], inner_he1,
                                 new_vertices[last_of_n], inner_e, f);
        outer_he1->set_neighbors(branch_he1, outer_he1->twin(), outer_he1->vertex(),
                                 outer_he1->edge(), side_f);
        branch_he1->set_neighbors(inner_he1, branch_he2, old_v, branch_e, side_f);
        branch_he2->set_neighbors(outer_halfedges[next_of_2n], branch_he1, new_v, branch_e,
                                  side_faces[next_of_n]);
    }
    return f;
}

/*
    Compute new vertex positions for the vertices of the beveled vertex.
    These vertices can be accessed via new_halfedges[i]->vertex()->pos for
    i = 1, ..., new_halfedges.size()-1.
    The basic strategy here is to loop over the list of outgoing halfedges,
    and use the original vertex position and its associated outgoing edge
    to compute a new vertex position along the outgoing edge.
*/
void Halfedge_Mesh::bevel_vertex_positions(const std::vector<Vec3>& start_positions,
                                           Halfedge_Mesh::FaceRef face, float tangent_offset) {

    std::vector<HalfedgeRef> new_halfedges;
    auto h = face->halfedge();
    do {
        new_halfedges.push_back(h);
        h = h->next();
    } while(h != face->halfedge());

    // (void)new_halfedges;
    (void)start_positions;
    (void)face;
    (void)tangent_offset;
}

/*
    Compute new vertex positions for the vertices of the beveled edge.
    These vertices can be accessed via new_halfedges[i]->vertex()->pos for
    i = 1, ..., new_halfedges.size()-1.
    The basic strategy here is to loop over the list of outgoing halfedges,
    and use the preceding and next vertex position from the original mesh
    (in the orig array) to compute an offset vertex position.
    Note that there is a 1-to-1 correspondence between halfedges in
    newHalfedges and vertex positions
    in orig.  So, you can write loops of the form
    for(size_t i = 0; i < new_halfedges.size(); i++)
    {
            Vector3D pi = start_positions[i]; // get the original vertex
            position corresponding to vertex i
    }
*/
void Halfedge_Mesh::bevel_edge_positions(const std::vector<Vec3>& start_positions,
                                         Halfedge_Mesh::FaceRef face, float tangent_offset) {

    std::vector<HalfedgeRef> new_halfedges;
    auto h = face->halfedge();
    do {
        new_halfedges.push_back(h);
        h = h->next();
    } while(h != face->halfedge());

    (void)new_halfedges;
    (void)start_positions;
    (void)face;
    (void)tangent_offset;
}

/*
    Compute new vertex positions for the vertices of the beveled face.
    These vertices can be accessed via new_halfedges[i]->vertex()->pos for
    i = 1, ..., new_halfedges.size()-1.
    The basic strategy here is to loop over the list of outgoing halfedges,
    and use the preceding and next vertex position from the original mesh
    (in the start_positions array) to compute an offset vertex
    position.
    Note that there is a 1-to-1 correspondence between halfedges in
    new_halfedges and vertex positions
    in orig. So, you can write loops of the form
    for(size_t i = 0; i < new_halfedges.size(); i++)
    {
            Vec3 pi = start_positions[i]; // get the original vertex
            position corresponding to vertex i
    }
*/
void Halfedge_Mesh::bevel_face_positions(const std::vector<Vec3>& start_positions,
                                         Halfedge_Mesh::FaceRef face, float tangent_offset,
                                         float normal_offset) {

    if(flip_orientation) normal_offset = -normal_offset;
    std::vector<HalfedgeRef> new_halfedges;
    auto h = face->halfedge();
    do {
        new_halfedges.push_back(h);
        h = h->next();
    } while(h != face->halfedge());

    Vec3 norm = face->normal();
    Vec3 c = face->center();
    size_t size = new_halfedges.size();
    // Iterate over the list of halfedges (new_halfedges)
    for(size_t i = 0; i < size; i++) {

        // get the original vertex
        Vec3 pi = start_positions[i];

        // Compute the updated vertex positions using the current values of tangent_offset (and
        // possibly normal_offset)
        Vec3 distance_to_start = pi - c;
        Vec3 with_tan = distance_to_start * tangent_offset + pi;
        Vec3 with_norm = normal_offset * norm + with_tan;

        // update position
        new_halfedges[i]->vertex()->pos = with_norm;
    }
}

/*
    Splits all non-triangular faces into triangles.
*/
void Halfedge_Mesh::triangulate() {
    // every n-polygon requires n-3 internal edges to triangulate and n-2 faces.
    for(FaceRef f0 = faces_begin(); f0 != faces_end(); f0++) {
        int n = (int)f0->degree();
        if(n == 3) {
            continue;
        }

        std::vector<HalfedgeRef> old_halfedges, new_halfedges;
        std::vector<EdgeRef> old_edges, new_edges;
        std::vector<VertexRef> old_vertices;
        std::vector<FaceRef> new_faces;
        auto h = f0->halfedge();
        do {
            old_halfedges.push_back(h);
            old_edges.push_back(h->edge());
            old_vertices.push_back(h->vertex());
            h = h->next();
        } while(h != f0->halfedge());

        // create new faces
        for(int i = 0; i < n - 2; i++) {
            new_faces.push_back(new_face());
        }

        // create new edges and half_edges
        for(int i = 0; i < n - 3; i++) {
            new_edges.push_back(new_edge());
            new_halfedges.push_back(new_halfedge());
            new_halfedges.push_back(new_halfedge());
        }

        VertexRef v0 = old_vertices[0];
        for(int i = 0; i < n - 2; i++) {
            HalfedgeRef h_next = h->next();
            HalfedgeRef h_next2 = old_halfedges[i + 2];
            if(i != n - 2 - 1) {
                EdgeRef e = new_edges[i];
                HalfedgeRef e_h1 = new_halfedges[i * 2];
                HalfedgeRef e_h2 = new_halfedges[i * 2 + 1];
                FaceRef f = new_faces[i];
                VertexRef v = h_next2->vertex();

                f->halfedge() = h;
                e->halfedge() = e_h1;

                h->set_neighbors(h_next, h->twin(), v0, h->edge(), f);
                h_next->set_neighbors(e_h1, h_next->twin(), h_next->vertex(), h_next->edge(), f);
                e_h1->set_neighbors(h, e_h2, v, e, f);
                e_h2->set_neighbors(h_next2, e_h1, v0, e, new_faces[i + 1]);

                h = e_h2;
            } else { // last triangle has an old edge
                FaceRef f = new_faces[i];

                f->halfedge() = h;

                h->set_neighbors(h_next, h->twin(), v0, h->edge(), f);
                h_next->set_neighbors(h_next2, h_next->twin(), h_next->vertex(), h_next->edge(), f);
                h_next2->set_neighbors(h, h_next2->twin(), h_next2->vertex(), h_next2->edge(), f);
            }
        }

        erase(f0);
    }
}

/* Note on the quad subdivision process:
        Unlike the local mesh operations (like bevel or edge flip), we will perform
        subdivision by splitting *all* faces into quads "simultaneously."  Rather
        than operating directly on the halfedge data structure (which as you've
        seen is quite difficult to maintain!) we are going to do something a bit nicer:
           1. Create a raw list of vertex positions and faces (rather than a full-
              blown halfedge mesh).
           2. Build a new halfedge mesh from these lists, replacing the old one.
        Sometimes rebuilding a data structure from scratch is simpler (and even
        more efficient) than incrementally modifying the existing one.  These steps are
        detailed below.
  Step I: Compute the vertex positions for the subdivided mesh.
        Here we're going to do something a little bit strange: since we will
        have one vertex in the subdivided mesh for each vertex, edge, and face in
        the original mesh, we can nicely store the new vertex *positions* as
        attributes on vertices, edges, and faces of the original mesh. These positions
        can then be conveniently copied into the new, subdivided mesh.
        This is what you will implement in linear_subdivide_positions() and
        catmullclark_subdivide_positions().
  Steps II-IV are provided (see Halfedge_Mesh::subdivide()), but are still detailed
  here:
  Step II: Assign a unique index (starting at 0) to each vertex, edge, and
        face in the original mesh. These indices will be the indices of the
        vertices in the new (subdivided mesh).  They do not have to be assigned
        in any particular order, so long as no index is shared by more than one
        mesh element, and the total number of indices is equal to V+E+F, i.e.,
        the total number of vertices plus edges plus faces in the original mesh.
        Basically we just need a one-to-one mapping between original mesh elements
        and subdivided mesh vertices.
  Step III: Build a list of quads in the new (subdivided) mesh, as tuples of
        the element indices defined above. In other words, each new quad should be
        of the form (i,j,k,l), where i,j,k and l are four of the indices stored on
        our original mesh elements.  Note that it is essential to get the orientation
        right here: (i,j,k,l) is not the same as (l,k,j,i).  Indices of new faces
        should circulate in the same direction as old faces (think about the right-hand
        rule).
  Step IV: Pass the list of vertices and quads to a routine that clears
        the internal data for this halfedge mesh, and builds new halfedge data from
        scratch, using the two lists.
*/

/*
    Compute new vertex positions for a mesh that splits each polygon
    into quads (by inserting a vertex at the face midpoint and each
    of the edge midpoints).  The new vertex positions will be stored
    in the members Vertex::new_pos, Edge::new_pos, and
    Face::new_pos.  The values of the positions are based on
    simple linear interpolation, e.g., the edge midpoints and face
    centroids.
*/
void Halfedge_Mesh::linear_subdivide_positions() {

    // For each vertex, assign Vertex::new_pos to
    // its original position, Vertex::pos.
    for(VertexRef v = vertices_begin(); v != vertices_end(); v++) {
        v->new_pos = v->pos;
    }

    // For each edge, assign the midpoint of the two original
    // positions to Edge::new_pos.
    for(EdgeRef e = edges_begin(); e != edges_end(); e++) {
        e->new_pos = e->center();
    }

    // For each face, assign the centroid (i.e., arithmetic mean)
    // of the original vertex positions to Face::new_pos. Note
    // that in general, NOT all faces will be triangles!
    for(FaceRef f = faces_begin(); f != faces_end(); f++) {
        f->new_pos = f->center();
    }
}

/*
    Compute new vertex positions for a mesh that splits each polygon
    into quads (by inserting a vertex at the face midpoint and each
    of the edge midpoints).  The new vertex positions will be stored
    in the members Vertex::new_pos, Edge::new_pos, and
    Face::new_pos.  The values of the positions are based on
    the Catmull-Clark rules for subdivision.
    Note: this will only be called on meshes without boundary
*/
void Halfedge_Mesh::catmullclark_subdivide_positions() {

    // The implementation for this routine should be
    // a lot like Halfedge_Mesh:linear_subdivide_positions:(),
    // except that the calculation of the positions themsevles is
    // slightly more involved, using the Catmull-Clark subdivision
    // rules. (These rules are outlined in the Developer Manual.)

    // Faces
    for(FaceRef f = faces_begin(); f != faces_end(); f++) {
        Vec3 result;
        HalfedgeRef h = f->halfedge();
        do {
            result += h->vertex()->pos;
            h = h->next();
        } while(h != f->halfedge());
        f->new_pos = result / f->degree();
    }

    // Edges
    for(EdgeRef e = edges_begin(); e != edges_end(); e++) {
        FaceRef f1 = e->halfedge()->face();
        FaceRef f2 = e->halfedge()->twin()->face();

        Vec3 avg = (f1->new_pos + f2->new_pos) / 2;
        e->new_pos = avg;
    }

    // Vertices
    for(VertexRef v = vertices_begin(); v != vertices_end(); v++) {
        unsigned int n = v->degree();
        Vec3 q, r;
        HalfedgeRef h = v->halfedge();
        do {
            q += h->face()->new_pos;
            r += h->edge()->new_pos;
            h = h->twin()->next();
        } while(h != v->halfedge());
        q /= n;
        r /= n;
        Vec3 s = v->pos;

        v->new_pos = (q + 2 * r + (n - 3) * s) / n;
    }
}

/*
        This routine should increase the number of triangles in the mesh
        using Loop subdivision. Note: this is will only be called on triangle meshes.
*/
void Halfedge_Mesh::loop_subdivide() {

    // Compute new positions for all the vertices in the input mesh, using
    // the Loop subdivision rule, and store them in Vertex::new_pos.
    // -> At this point, we also want to mark each vertex as being a vertex of the
    //    original mesh. Use Vertex::is_new for this.
    // -> Next, compute the updated vertex positions associated with edges, and
    //    store it in Edge::new_pos.
    // -> Next, we're going to split every edge in the mesh, in any order.  For
    //    future reference, we're also going to store some information about which
    //    subdivided edges come from splitting an edge in the original mesh, and
    //    which edges are new, by setting the flat Edge::is_new. Note that in this
    //    loop, we only want to iterate over edges of the original mesh.
    //    Otherwise, we'll end up splitting edges that we just split (and the
    //    loop will never end!)
    // -> Now flip any new edge that connects an old and new vertex.
    // -> Finally, copy the new vertex positions into final Vertex::pos.

    // Each vertex and edge of the original surface can be associated with a
    // vertex in the new (subdivided) surface.
    // Therefore, our strategy for computing the subdivided vertex locations is to
    // *first* compute the new positions
    // using the connectivity of the original (coarse) mesh; navigating this mesh
    // will be much easier than navigating
    // the new subdivided (fine) mesh, which has more elements to traverse.  We
    // will then assign vertex positions in
    // the new mesh based on the values we computed for the original mesh.

    // Compute updated positions for all the vertices in the original mesh, using
    // the Loop subdivision rule.
    for(VertexRef v = vertices_begin(); v != vertices_end(); v++) {
        int n = (int)v->degree();
        float u = (n == 3) ? 3.f / 16.f : 3.f / (8.f * n);
        Vec3 sum_neighbors;
        HalfedgeRef h = v->halfedge();
        do {
            sum_neighbors += h->twin()->vertex()->pos;
            h = h->twin()->next();
        } while(h != v->halfedge());
        v->new_pos = ((1.f - n * u) * v->pos) + (u * sum_neighbors);
        v->is_new = false;
    }

    // Next, compute the updated vertex positions associated with edges.
    for(EdgeRef e = edges_begin(); e != edges_end(); e++) {
        HalfedgeRef h = e->halfedge();
        HalfedgeRef twin = h->twin();

        Vec3 a, b, c, d;
        a = h->vertex()->new_pos;
        b = twin->vertex()->new_pos;
        c = h->next()->next()->vertex()->new_pos;
        d = twin->next()->next()->vertex()->new_pos;
        e->new_pos = (3.f / 8.f) * (a + b) + (1.f / 8.f) * (c + d);
    }

    // Next, we're going to split every edge in the mesh, in any order. For
    // future reference, we're also going to store some information about which
    // subdivided edges come from splitting an edge in the original mesh, and
    // which edges are new.
    // In this loop, we only want to iterate over edges of the original
    // mesh---otherwise, we'll end up splitting edges that we just split (and
    // the loop will never end!)
    int n = n_edges();
    EdgeRef e = edges_begin();
    for(int i = 0; i < n; i++) {

        EdgeRef nextEdge = e;
        nextEdge++;

        // split edge marks is_new for new edges and new vertices
        VertexRef v1 = split_edge(e).value();
        v1->new_pos = e->new_pos;

        e = nextEdge;
    }

    // Finally, flip any new edge that connects an old and new vertex.
    for(EdgeRef e = edges_begin(); e != edges_end(); e++) {
        if(e->is_new) {
            bool v1_new = e->halfedge()->vertex()->is_new;
            bool v2_new = e->halfedge()->twin()->vertex()->is_new;

            if((!v1_new && v2_new) || (v1_new && !v2_new)) {
                flip_edge(e).value();
            }
        }
    }

    // Copy the updated vertex positions to the subdivided mesh.
    for(VertexRef v = vertices_begin(); v != vertices_end(); v++) {
        v->pos = v->new_pos;
    }
}

/*
    Isotropic remeshing. Note that this function returns success in a similar
    manner to the local operations, except with only a boolean value.
    (e.g. you may want to return false if this is not a triangle mesh)
*/
bool Halfedge_Mesh::isotropic_remesh() {

    // Compute the mean edge length.
    // Repeat the four main steps for 5 or 6 iterations
    // -> Split edges much longer than the target length (being careful about
    //    how the loop is written!)
    // -> Collapse edges much shorter than the target length.  Here we need to
    //    be EXTRA careful about advancing the loop, because many edges may have
    //    been destroyed by a collapse (which ones?)
    // -> Now flip each edge if it improves vertex degree
    // -> Finally, apply some tangential smoothing to the vertex positions

    // Note: if you erase elements in a local operation, they will not be actually deleted
    // until do_erase or validate are called. This is to facilitate checking
    // for dangling references to elements that will be erased.
    // The rest of the codebase will automatically call validate() after each op,
    // but here simply calling collapse_edge() will not erase the elements.
    // You should use collapse_edge_erase() instead for the desired behavior.

    for(FaceRef f0 = faces_begin(); f0 != faces_end(); f0++) {
        int n = (int)f0->degree();
        if(n == 3) {
            continue;
        }
    }
    int n = n_edges();

    float mean = 0.f;
    for(EdgeRef e = edges_begin(); e != edges_end(); e++) {
        mean += e->length();
    }
    mean /= n;

    // Split edges much longer than the target length
    EdgeRef e = edges_begin();
    for(int i = 0; i < n; i++) {

        EdgeRef nextEdge = e;
        nextEdge++;
        if(e->length() > (4.f / 3.f) * mean) {
            split_edge(e);
        }

        e = nextEdge;
    }

    // Collapse edges much shorter than the target length.
    e = edges_begin();
    for(int i = 0; i < n; i++) {
        EdgeRef nextEdge = e;
        nextEdge++;
        if(e->length() < 0.8f * mean) {
            collapse_edge_erase(e);
        }

        e = nextEdge;
    }

    // flip each edge if it improves vertex degree
    e = edges_begin();
    for(int i = 0; i < n; i++) {

        EdgeRef nextEdge = e;
        nextEdge++;

        HalfedgeRef h = e->halfedge();
        HalfedgeRef twin = h->twin();

        // a and b are the two vertices of the edge
        // c and d are opposite
        // triangles are abc and abd
        int a, b, c, d;
        a = (int)h->vertex()->degree();
        b = (int)twin->vertex()->degree();
        c = (int)h->next()->next()->vertex()->degree();
        d = (int)twin->next()->next()->vertex()->degree();

        // calculate deviation before flipping
        int pre_dev = std::abs(a - 6) + std::abs(b - 6) + std::abs(c - 6) + std::abs(d - 6);

        a--;
        b--;
        c++;
        d++;
        // calculate deviation after flipping
        int post_dev = std::abs(a - 6) + std::abs(b - 6) + std::abs(c - 6) + std::abs(d - 6);
        if(post_dev < pre_dev) {
            flip_edge(e);
        }

        e = nextEdge;
    }

    // apply some tangential smoothing to the vertex positions
    for(VertexRef v = vertices_begin(); v != vertices_end(); v++) {
        Vec3 c = v->neighborhood_center();
        Vec3 p = v->pos;

        v->new_pos = p + 0.2f * (c - p);
    }

    for(VertexRef v = vertices_begin(); v != vertices_end(); v++) {
        v->pos = v->new_pos;
    }

    return true;
}

/* Helper type for quadric simplification */
struct Edge_Record {
    Edge_Record() {
    }
    Edge_Record(std::unordered_map<Halfedge_Mesh::VertexRef, Mat4>& vertex_quadrics,
                Halfedge_Mesh::EdgeRef e)
        : edge(e) {

        // Compute the combined quadric from the edge endpoints.
        Mat4 K = vertex_quadrics[e->halfedge()->vertex()] +
                 vertex_quadrics[e->halfedge()->twin()->vertex()];

        // -> Build the 3x3 linear system whose solution minimizes the quadric error
        //    associated with these two endpoints.
        Mat4 A = K.axes(K.cols[0].xyz(), K.cols[1].xyz(), K.cols[2].xyz());
        Vec3 b = -1 * K[3].xyz();

        // -> Use this system to solve for the optimal position, and store it in
        //    Edge_Record::optimal.
        // check if A is nearly singular (det (A) is roughly 0) - per the Garland paper
        if(abs(A.det()) < 1e-5) {
            optimal = A.inverse() * b;
        } else {
            optimal = e->center();
        }

        // -> Also store the cost associated with collapsing this edge in
        //    Edge_Record::cost.
        Vec4 x = Vec4(optimal, 1);
        cost = dot(x, K * x);
    }
    Halfedge_Mesh::EdgeRef edge;
    Vec3 optimal;
    float cost;
};

/* Comparison operator for Edge_Records so std::set will properly order them */
bool operator<(const Edge_Record& r1, const Edge_Record& r2) {
    if(r1.cost != r2.cost) {
        return r1.cost < r2.cost;
    }
    Halfedge_Mesh::EdgeRef e1 = r1.edge;
    Halfedge_Mesh::EdgeRef e2 = r2.edge;
    return &*e1 < &*e2;
}

/** Helper type for quadric simplification
 *
 * A PQueue is a minimum-priority queue that
 * allows elements to be both inserted and removed from the
 * queue.  Together, one can easily change the priority of
 * an item by removing it, and re-inserting the same item
 * but with a different priority.  A priority queue, for
 * those who don't remember or haven't seen it before, is a
 * data structure that always keeps track of the item with
 * the smallest priority or "score," even as new elements
 * are inserted and removed.  Priority queues are often an
 * essential component of greedy algorithms, where one wants
 * to iteratively operate on the current "best" element.
 *
 * PQueue is templated on the type T of the object
 * being queued.  For this reason, T must define a comparison
 * operator of the form
 *
 *    bool operator<( const T& t1, const T& t2 )
 *
 * which returns true if and only if t1 is considered to have a
 * lower priority than t2.
 *
 * Basic use of a PQueue might look
 * something like this:
 *
 *    // initialize an empty queue
 *    PQueue<myItemType> queue;
 *
 *    // add some items (which we assume have been created
 *    // elsewhere, each of which has its priority stored as
 *    // some kind of internal member variable)
 *    queue.insert( item1 );
 *    queue.insert( item2 );
 *    queue.insert( item3 );
 *
 *    // get the highest priority item currently in the queue
 *    myItemType highestPriorityItem = queue.top();
 *
 *    // remove the highest priority item, automatically
 *    // promoting the next-highest priority item to the top
 *    queue.pop();
 *
 *    myItemType nextHighestPriorityItem = queue.top();
 *
 *    // Etc.
 *
 *    // We can also remove an item, making sure it is no
 *    // longer in the queue (note that this item may already
 *    // have been removed, if it was the 1st or 2nd-highest
 *    // priority item!)
 *    queue.remove( item2 );
 *
 */
template<class T> struct PQueue {
    void insert(const T& item) {
        queue.insert(item);
    }
    void remove(const T& item) {
        if(queue.find(item) != queue.end()) {
            queue.erase(item);
        }
    }
    const T& top(void) const {
        return *(queue.begin());
    }
    void pop(void) {
        queue.erase(queue.begin());
    }
    size_t size() {
        return queue.size();
    }

    std::set<T> queue;
};

/*
    Mesh simplification. Note that this function returns success in a similar
    manner to the local operations, except with only a boolean value.
    (e.g. you may want to return false if you can't simplify the mesh any
    further without destroying it.)
*/
bool Halfedge_Mesh::simplify() {

    std::unordered_map<VertexRef, Mat4> vertex_quadrics;
    std::unordered_map<FaceRef, Mat4> face_quadrics;
    std::unordered_map<EdgeRef, Edge_Record> edge_records;
    PQueue<Edge_Record> edge_queue;

    // Compute initial quadrics for each face by simply writing the plane equation
    // for the face in homogeneous coordinates. These quadrics should be stored
    // in face_quadrics
    int num_faces = 0;
    for(FaceRef face = faces_begin(); face != faces_end(); ++face) {
        Vec3 N = face->normal();
        Vec3 p = face->center();
        float d = dot(N, p) * -1;
        Vec4 v(N, d);
        face_quadrics[face] = outer(v, v);

        num_faces += 1;
    }

    // -> Compute an initial quadric for each vertex as the sum of the quadrics
    //    associated with the incident faces, storing it in vertex_quadrics
    for(VertexRef vertex = vertices_begin(); vertex != vertices_end(); ++vertex) {
        Mat4 face_quadric_sum;
        HalfedgeRef vertex_halfedge = vertex->halfedge();
        face_quadric_sum += face_quadrics[vertex_halfedge->face()];

        HalfedgeRef h_iter = vertex_halfedge->twin()->next();
        while(h_iter != vertex_halfedge) {
            face_quadric_sum += face_quadrics[h_iter->face()];
            h_iter = h_iter->twin()->next();
        }
        vertex_quadrics[vertex] = face_quadric_sum;
    }

    // -> Build a priority queue of edges according to their quadric error cost,
    //    i.e., by building an Edge_Record for each edge and sticking it in the
    //    queue. You may want to use the above PQueue<Edge_Record> for this.
    for(EdgeRef e = edges_begin(); e != edges_end(); e++) {
        Edge_Record record = Edge_Record(vertex_quadrics, e);
        edge_records[e] = record;
        edge_queue.insert(record);
    }

    // -> Until we reach the target edge budget, collapse the best edge. Remember
    //    to remove from the queue any edge that touches the collapsing edge
    //    BEFORE it gets collapsed, and add back into the queue any edge touching
    //    the collapsed vertex AFTER it's been collapsed. Also remember to assign
    //    a quadric to the collapsed vertex, and to pop the collapsed edge off the
    //    top of the queue.
    int target_edge_budget = 0.25 * num_faces;
    for(int i = 0; i < target_edge_budget; ++i) {
        // 1. Get the cheapest edge from the queue.
        Edge_Record cheapest_edge_record = edge_queue.top();
        EdgeRef cheapest_edge = cheapest_edge_record.edge;

        // 2. Remove the cheapest edge from the queue by calling pop().
        edge_queue.pop();

        // 3. Compute the new quadric by summing the quadrics at its two endpoints.
        VertexRef vertex_endpoint_1 = cheapest_edge->halfedge()->vertex();
        VertexRef vertex_endpoint_2 = cheapest_edge->halfedge()->twin()->vertex();
        Mat4 new_quadric = vertex_quadrics[vertex_endpoint_1] +
                           vertex_quadrics[cheapest_edge->halfedge()->twin()->vertex()];

        // 4. Remove any edge touching either of its endpoints from the queue.
        // endpoint one
        HalfedgeRef vertex_endpoint_1_halfedge = vertex_endpoint_1->halfedge();
        EdgeRef edge_to_remove = vertex_endpoint_1_halfedge->edge();
        Edge_Record record_to_remove = Edge_Record(vertex_quadrics, edge_to_remove);
        edge_records.erase(edge_to_remove);
        edge_queue.remove(record_to_remove);

        HalfedgeRef h_iter = vertex_endpoint_1_halfedge->twin()->next();
        while(h_iter != vertex_endpoint_1_halfedge) {
            edge_to_remove = h_iter->edge();
            record_to_remove = Edge_Record(vertex_quadrics, edge_to_remove);
            edge_records.erase(edge_to_remove);
            edge_queue.remove(record_to_remove);

            h_iter = h_iter->twin()->next();
        }

        // endpoint two
        HalfedgeRef vertex_endpoint_2_halfedge = vertex_endpoint_2->halfedge();
        edge_to_remove = vertex_endpoint_2_halfedge->edge();
        record_to_remove = Edge_Record(vertex_quadrics, edge_to_remove);
        edge_queue.remove(record_to_remove);

        h_iter = vertex_endpoint_2_halfedge->twin()->next();
        while(h_iter != vertex_endpoint_2_halfedge) {
            edge_to_remove = h_iter->edge();
            record_to_remove = Edge_Record(vertex_quadrics, edge_to_remove);
            edge_queue.remove(record_to_remove);

            h_iter = h_iter->twin()->next();
        }

        // 5. Collapse the edge.
        // Note: if you erase elements in a local operation, they will not be actually deleted
        // until do_erase or validate are called. This is to facilitate checking
        // for dangling references to elements that will be erased.
        // The rest of the codebase will automatically call validate() after each op,
        // but here simply calling collapse_edge() will not erase the elements.
        // You should use collapse_edge_erase() instead for the desired behavior.
        std::optional<VertexRef> optional_remaining_vertex = collapse_edge_erase(cheapest_edge);
        if(!optional_remaining_vertex.has_value()) {
            return false;
        }
        VertexRef remaining_vertex = optional_remaining_vertex.value();

        // 6. Set the quadric of the new vertex to the quadric computed in step 4.
        vertex_quadrics[remaining_vertex] = new_quadric;

        // 7. Insert any edge touching the new vertex into the queue, creating new edge records for
        // each of them.
        HalfedgeRef remaining_vertex_halfedge = remaining_vertex->halfedge();
        EdgeRef new_edge = remaining_vertex_halfedge->edge();
        Edge_Record record = Edge_Record(vertex_quadrics, new_edge);
        edge_records[new_edge] = record;
        edge_queue.insert(record);

        h_iter = remaining_vertex_halfedge->twin()->next();
        while(h_iter != remaining_vertex_halfedge) {
            new_edge = h_iter->edge();
            record = Edge_Record(vertex_quadrics, new_edge);
            edge_records[new_edge] = record;
            edge_queue.insert(record);

            h_iter = h_iter->twin()->next();
        }
    }

    return true;
}