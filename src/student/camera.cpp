
#include "../util/camera.h"
#include "../rays/samplers.h"
#include "debug.h"

Ray Camera::generate_ray(Vec2 screen_coord) const {

    // TODO (PathTracer): Task 1
    //
    // The input screen_coord is a normalized screen coordinate [0,1]^2
    //
    // You need to transform this 2D point into a 3D position on the sensor plane, which is
    // located one unit away from the pinhole in camera space (aka view space).
    //
    // You'll need to compute this position based on the vertial field of view
    // (vert_fov) of the camera, and the aspect ratio of the output image (aspect_ratio).
    //
    // Tip: compute the ray direction in view space and use
    // the camera space to world space transform (iview) to transform the ray back into world space.
    float screen_h = 2 * std::tan(Radians(vert_fov) / 2);
    float screen_w = screen_h * aspect_ratio;

    float offset_h = screen_h / 2;
    float offset_w = screen_w / 2;
    
    // unit direction looking down -z axis
    Vec3 dir =
        Vec3(screen_w * screen_coord.x - offset_w, screen_h * screen_coord.y - offset_h, -1.0f)
            .unit();

    // ray beginning at origin
    Ray ray(Vec3(0, 0, 0), dir);

    // transform using iview into world space
    ray.transform(iview);

    return ray;
}
