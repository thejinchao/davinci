#pragma once

#include "dvr_triangle.h"

typedef std::function<void(int32_t i, const Vector3&, const Vector3&, const Vector3&)> ResultCallback;

void buildDelaunayTriangles(int32_t canvasWidth, int32_t canvasHeight, size_t pointCounts, TriangleVector& triangles);
