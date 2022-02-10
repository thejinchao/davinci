#pragma once

#include "dv_prerequisites.h"

#include "dv_pixel_buffer.h"

namespace davinci
{

class RenderTarget
{
public:
	void init(int width, int height);
	void setPixel(int32_t x, int32_t y, const Vector4& color, float depth);

	int32_t getWidth(void) const { return m_width; }
	int32_t getHeight(void) const { return m_height; }
	const PixelBuffer<Vector4>& getColorBuffer(void) const { return m_colorBuffer; }
	const PixelBuffer<float>& getDepthBuffer(void) const { return m_depthBuffer; }

private:
	int32_t m_width;
	int32_t m_height;
	PixelBuffer<Vector4> m_colorBuffer;
	PixelBuffer<float> m_depthBuffer;

public:
	RenderTarget();
	~RenderTarget() {}
};

}