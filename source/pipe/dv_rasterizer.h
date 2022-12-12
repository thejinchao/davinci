#pragma once

#include "dv_prerequisites.h"

namespace davinci
{

class Rasterizer
{
	//Draw Line
public:
	typedef std::function<void(const std::pair<int32_t, int32_t>&, float)> DrawLineCallback;

	//Bresenham's line algorithm
	static void drawLine(int32_t canvasWidth, int32_t canvasHeight, 
		const fVector2& start, const fVector2& end, 
		DrawLineCallback callback);

	//Draw Triangle
public:
	typedef std::function<void(const std::pair<int32_t, int32_t>&, const fVector3&)> DrawTriangleCallback;

	struct DebugParam
	{
		bool debug;
		int32_t x;
		int32_t y;
		int32_t tile_id;
		int32_t coarse_id;
		int32_t fine_id;
		int32_t edge_id;
	};

	//Larrabee algorithm
	static void drawTriangleLarrabee(int32_t canvasWidth, int32_t canvasHeight, 
		const fVector3& v0, const fVector3& v1, const fVector3& v2, 
		DrawTriangleCallback callback, 
		const DebugParam* debug=nullptr);

	//Scaleline algorithm
	static void drawTriangleScanline(int32_t canvasWidth, int32_t canvasHeight,
		const fVector3& v0, const fVector3& v1, const fVector3& v2,
		DrawTriangleCallback callback);
};

}