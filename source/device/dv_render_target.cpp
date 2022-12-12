#include "dv_precompiled.h"
#include "dv_render_target.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
RenderTarget::RenderTarget()
{
}

//-------------------------------------------------------------------------------------
void RenderTarget::init(int width, int height)
{
	m_width = width;
	m_height = height;
	m_colorBuffer.init(width, height, fVector4::BLACK);
	m_depthBuffer.init(width, height, std::numeric_limits<float>::max());
}

//-------------------------------------------------------------------------------------
void RenderTarget::setPixel(int32_t x, int32_t y, const fVector4& color, float depth)
{
	if (x < 0 || x >= m_width || y<0 || y>=m_height) return;

	if (depth > m_depthBuffer.getPixel(x, y)) {
		return;
	}
	m_colorBuffer.setPixel(x, y, color);
	m_depthBuffer.setPixel(x, y, depth);
}

}
