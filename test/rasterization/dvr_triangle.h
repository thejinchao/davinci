#pragma once

#include <davinci.h>
using namespace davinci;

struct Triangle
{
public:
	fVector3 p0, p1, p2;
	fVector3 pixelColor;
	std::vector< std::pair<int32_t, int32_t> > pixels;

public:
	Triangle(){}
	Triangle(const fVector3& _p0, const fVector3& _p1, const fVector3& _p2) : p0(_p0), p1(_p1), p2(_p2) { }
	Triangle(const Triangle& other) : p0(other.p0), p1(other.p1), p2(other.p2), pixelColor(other.pixelColor), pixels(other.pixels) {}

public:
	void build(const fVector3& _pixelColor, int32_t canvasWidth, int32_t canvasHeight, bool debug = false);
};

typedef std::vector< Triangle > TriangleVector;
