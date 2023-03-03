
#include "../lib/mathlib.h"
#include "debug.h"
#include <iostream>

bool BBox::hit(const Ray& ray, Vec2& times) const {

    // TODO (PathTracer):
    // Implement ray - bounding box intersection test
    // If the ray intersected the bounding box within the range given by
    // [times.x,times.y], update times with the new intersection times.

    Vec3 o = ray.point;
    Vec3 d = ray.dir;

    Vec3 isect_min = (min - o) / d;
    Vec3 isect_max = (max - o) / d;

    if(isect_min.x > isect_max.x) {
        std::swap(isect_min.x, isect_max.x);
    }
    if(isect_min.y > isect_max.y) {
        std::swap(isect_min.y, isect_max.y);
    }
    if(isect_min.z > isect_max.z) {
        std::swap(isect_min.z, isect_max.z);
    }

    float t_min = std::max(std::max(isect_min.x, isect_min.y), isect_min.z);
    float t_max = std::min(std::min(isect_max.x, isect_max.y), isect_max.z);

    if(t_min > t_max) {
        return false;
    }

    times.x = t_min;
    times.y = t_max;

    return true;
}
