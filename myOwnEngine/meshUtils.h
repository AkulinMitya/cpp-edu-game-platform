#ifndef MESHUTILS_H
#define MESHUTILS_H

#include <vector>
#include "point/point.h"

inline std::vector<Point> makeRectangleMesh(float width, float height) {
    float w2 = width * 0.5f;
    float h2 = height * 0.5f;
    return {
        {-w2, -h2},
        { w2, -h2},
        { w2,  h2},
        {-w2,  h2}
    };
}

#endif // MESHUTILS_H
