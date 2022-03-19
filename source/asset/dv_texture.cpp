#include "dv_precompiled.h"
#include "dv_texture.h"

namespace davinci
{

//-------------------------------------------------------------------------------------
Texture::Texture(int32_t width, int32_t height, PixelFormat pixelFormat)
	: m_width(width)
	, m_height(height)
	, m_pixelFormat(pixelFormat)
{
	m_pixelSize = getPixelSize(m_pixelFormat);

	m_pixelData.resize((size_t)(m_width*m_height*m_pixelSize));
}

//-------------------------------------------------------------------------------------
Texture::~Texture()
{

}

//-------------------------------------------------------------------------------------
int32_t Texture::getPixelSize(PixelFormat pixelFormat)
{
	switch (pixelFormat) {
	case PF_FLOAT32_RGB: return 3;
	case PF_FLOAT32_RGBA: return 4;
	default:
		assert(false);
		return 0;
	}
}

//-------------------------------------------------------------------------------------
Vector3 Texture::getRGB(const Vector2& uv) const
{
	float fx = uv.x - floorf(uv.x);
	float fy = uv.y - floorf(uv.y);

	int32_t x = ((int32_t)(fx * m_width)) % m_width;
	int32_t y = ((int32_t)(fy * m_height)) % m_height;
	size_t pixelPos = (size_t)((y*m_width + x)*m_pixelSize);

	return Vector3(&(m_pixelData[pixelPos]));
}

}