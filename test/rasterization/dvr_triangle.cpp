#include "dvr_triangle.h"
#include "dvr_rasterizer.h"

//-------------------------------------------------------------------------------------
static void larabeeTest(int32_t width, int32_t height, Vector3 v0, Vector3 v1, Vector3 v2, Rasterizer::DrawTriangleCallback callback)
{
#define s1516_flt(f) (int32_t)(f * 0xffff)

	static framebuffer_t* fb = nullptr;
	if (fb == nullptr) fb = new_framebuffer(width, height);

	static uint8_t* rgba8_pixels = nullptr;
	if (rgba8_pixels == nullptr) rgba8_pixels = (uint8_t*)malloc(width * height * 4);

	//clear
	framebuffer_clear(fb, 0x00000000);

	v0.x = v0.x / (width / 2.f) - 1.f;
	v0.y = v0.y / (height / 2.f) - 1.f;

	v1.x = v1.x / (width / 2.f) - 1.f;
	v1.y = v1.y / (height / 2.f) - 1.f;

	v2.x = v2.x / (width / 2.f) - 1.f;
	v2.y = v2.y / (height / 2.f) - 1.f;

	int xverts[3][4] = {
		{ s1516_flt(v0.x), s1516_flt(v0.y), s1516_flt(v0.z), s1516_flt(1.f) },
		{ s1516_flt(v1.x), s1516_flt(v1.y), s1516_flt(v1.z), s1516_flt(1.f) },
		{ s1516_flt(v2.x), s1516_flt(v2.y), s1516_flt(v2.z), s1516_flt(1.f) },
	};

	framebuffer_draw(fb, &xverts[0][0], 3);
	framebuffer_resolve(fb);

	//dump
	memset(rgba8_pixels, 0, width * height * 4);
	framebuffer_pack_row_major(fb, attachment_color0, 0, 0, width, height, pixelformat_r8g8b8a8_unorm, rgba8_pixels);

	for (int32_t i = 0; i < height; i++) {
		for (int32_t j = 0; j < width; j++) {
			uint32_t col = *((uint32_t*)(rgba8_pixels + width * 4 * i + j * 4));

			if (col != 0x00000000) {
				callback({ j, height - 1 - i }, Vector3(0, 0, 0));
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void Triangle::build(const Vector3& _pixelColor, int32_t canvasWidth, int32_t canvasHeight, bool debug) 
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
		[this](std::pair<int32_t, int32_t> pos, const Vector3& t) {
		pixels.push_back(pos);
	});
}
