#pragma once

#include "huynity/objectlib/collections/polyline.h"
#include "huynity/objectlib/operations/triangulate.h"
#include "huynity/objectlib/shapes/box3.h"
#include "huynity/objectlib/shapes/circle2.h"
#include "huynity/objectlib/shapes/disk2.h"
#include "huynity/objectlib/shapes/rectangle2.h"
#include "huynity/objectlib/shapes/sphere3.h"

#include <cstddef>

namespace huynity::objectlib {

struct TessellationOptions {
    double maxDeviation = 0.01;
    std::size_t maxPrimitiveCount = 1'000'000;
};

template <std::size_t dimension>
using PolylineResult = GeometryResult<Polyline<dimension>>;

[[nodiscard]] PolylineResult<2> sampleCircle(const Circle2& circle, TessellationOptions options = {});
[[nodiscard]] TessellationResult<2> tessellate(const Disk2& disk, TessellationOptions options = {});
[[nodiscard]] TessellationResult<2> tessellate(const Rectangle2& rectangle, TessellationOptions options = {});
[[nodiscard]] TessellationResult<3> tessellate(const Box3& box, TessellationOptions options = {});
[[nodiscard]] TessellationResult<3> tessellate(const Sphere3& sphere, TessellationOptions options = {});

} // namespace huynity::objectlib
