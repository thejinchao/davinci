#include "dvr_triangle.h"

//-------------------------------------------------------------------------------------
void Triangle::build(const fVector3& _pixelColor, int32_t canvasWidth, int32_t canvasHeight, bool debug) 
{
	pixelColor = _pixelColor;
	pixels.clear();

	//make sure it's cw
	bool ccw = false;
	if ((p2 - p1).crossProduct(p0 - p2).z > 0) {
		ccw = true;
	}

	//build pixel data
	Rasterizer::drawTriangleLarrabee(canvasWidth, canvasHeight,
		p0, (ccw ? p2 : p1), (ccw ? p1 : p2),
		[this](std::pair<int32_t, int32_t> pos, const fVector3& t) {
		pixels.push_back(pos);
	});
}
