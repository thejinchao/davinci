#pragma once

#include "dv_prerequisites.h"

namespace davinci
{

template<typename T>
class PixelBuffer : noncopyable
{
public:
	void init(int width, int height, const T& pixel) {
		assert(width > 0 && width < 0xFFFF);
		assert(height > 0 && height < 0xFFFF);

		m_width = width;
		m_height = height;
		m_pixelBuffer.resize((size_t)(width*height));
		clear(pixel);
	}

	void clear(const T& pixel) {
		std::fill(m_pixelBuffer.begin(), m_pixelBuffer.end(), pixel);
	}

	void setPixel(int32_t x, int32_t y, const T& pixel) {
		assert(x >= 0 && x < m_width);
		assert(y >= 0 && y < m_height);

		m_pixelBuffer[(size_t)(y*m_width + x)] = pixel;
	}

	const T& getPixel(int32_t x, int32_t y) {
		assert(x >= 0 && x < m_width);
		assert(y >= 0 && y < m_height);

		return m_pixelBuffer[(size_t)(y*m_width + x)];
	}

	const T* ptr(void) const {
		return &(m_pixelBuffer[0]);
	}

	int32_t getWidth(void) const { return m_width; }
	int32_t getHeight(void) const { return m_height; }

protected:
	std::vector<T> m_pixelBuffer;
	int32_t m_width;
	int32_t m_height;

public:
	PixelBuffer() : m_width(0), m_height(0) { }
};


}
