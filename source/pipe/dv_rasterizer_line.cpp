#include "dv_precompiled.h"
#include "dv_rasterizer.h"

namespace davinci
{

//-------------------------------------------------------------------------------------------------------------
static void inline _switch_to_octant_zero(int32_t octant, int32_t& cx, int32_t& cy)
{
	int32_t temp;
	switch (octant) {
	case 0: return;									// return (x, y);
	case 1: temp = cx; cx = cy; cy = temp; return;	// return (y, x)
	case 2: temp = -cx; cx = cy; cy = temp; return; // return (y, -x);
	case 3: cx = -cx; return;						// return (-x, y);
	case 4: cx = -cx; cy = -cy; return;				// return (-x, -y);
	case 5: temp = -cx; cx = -cy; cy = temp; return;// return (-y, -x);
	case 6: temp = cx; cx = -cy; cy = temp;	return;	// return (-y, x);
	case 7: cy = -cy; return;						// return (x, -y);
	default:
		assert(false);
	}
}

//-------------------------------------------------------------------------------------------------------------
static std::pair<int32_t, int32_t> inline _switch_from_octant_zero(int32_t octant, int32_t cx, int32_t cy)
{
	switch (octant) {
	case 0: return std::make_pair(cx, cy);
	case 1: return std::make_pair(cy, cx);
	case 2: return std::make_pair(-cy, cx);
	case 3: return std::make_pair(-cx, cy);
	case 4: return std::make_pair(-cx, -cy);
	case 5: return std::make_pair(-cy, -cx);
	case 6: return std::make_pair(cy, -cx);
	case 7: return std::make_pair(cx, -cy);
	default:
		assert(false);
	}
	return std::make_pair(0, 0);
}

//-------------------------------------------------------------------------------------------------------------
inline int32_t _get_octants(int32_t dx, int32_t dy)
{
	/*
	Octants:
	\2|1/
	3\|/0
	---+---
	4/|\7
	/5|6\
	*/
	if (dx >= 0) {
		if (dy >= 0) return dy >= dx ? 1 : 0;
		else return (-dy) >= dx ? 6 : 7;
	}
	else {
		if (dy >= 0) return dy >= (-dx) ? 2 : 3;
		else return dy <= dx ? 5 : 4;
	}
}

//-------------------------------------------------------------------------------------
void Rasterizer::drawLine(int32_t canvasWidth, int32_t canvasHeight, const fVector2& start, const fVector2& end, DrawLineCallback callback)
{
	int32_t cx1 = (int32_t)floor(start.x + 0.5f);
	int32_t cy1 = (int32_t)floor(start.y + 0.5f);

	int32_t cx2 = (int32_t)floor(end.x + 0.5f);
	int32_t cy2 = (int32_t)floor(end.y + 0.5f);

	int32_t dx = cx2 - cx1;
	int32_t dy = cy2 - cy1;
	int32_t octants = _get_octants(dx, dy);

	_switch_to_octant_zero(octants, cx1, cy1);
	_switch_to_octant_zero(octants, cx2, cy2);
	dx = cx2 - cx1;
	dy = cy2 - cy1;

	#define SET_PIXEL(x, y, p) callback(_switch_from_octant_zero(octants, x, y), p)

	if (dx == 0) {
		for (int32_t y = cy1; y <= cy2; y++) {
			float p = (float)(y - cy1) / (float)dy;
			SET_PIXEL(cx1, y, p);
		}
		return;
	}

	if (dy == 0) {
		for (int32_t x = cx1; x <= cx2; x++) {
			float p = (float)(x - cx1) / (float)dx;
			SET_PIXEL(x, cy1, p);
		}
		return;
	}

	int32_t D = (dy << 1) - dx;	//2*dy-dx;
	int32_t dy_dx_2 = (dy << 1) - (dx << 1);	//2*dy-2*dx;
	int32_t dy_2 = dy << 1;	//2*dy;

							//first pixel
	SET_PIXEL(cx1, cy1, 0.f);
	int32_t y = cy1;

	for (int x = cx1 + 1; x <= cx2; x++) {
		float p = (float)(x - cx1) / (float)dx;
		if (D > 0) {
			y++;
			SET_PIXEL(x, y, p);
			D += dy_dx_2;
		}
		else {
			SET_PIXEL(x, y, p);
			D += dy_2;
		}
	}
}

}
